#include "function_ast.h"
#include "codegen.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include <set>
#include <string>
#include <vector>

// Forward declaration (defined in codegen.cpp)
CodeGen& getCodeGen();

// Counter for generating unique function names
static int fnCounter = 0;

// Declare (or get) malloc function in the module
static llvm::Function* getMalloc(CodeGen& cg) {
    if (auto* fn = cg.getModule().getFunction("malloc")) return fn;
    auto* mallocTy = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(cg.getContext()),
        {llvm::Type::getInt64Ty(cg.getContext())},
        /*isVarArg=*/false);
    return llvm::Function::Create(
        mallocTy, llvm::Function::ExternalLinkage, "malloc", cg.getModule());
}

// Recursively collect variable reads and local declaration names from an AST node.
// Does NOT recurse into nested FunctionLiteralAST (separate scope).
static void collectVarRefsAndDecls(ASTNode* node,
                                    std::vector<std::string>& refs,
                                    std::vector<std::string>& decls) {
    if (!node) return;
    if (auto* v = dynamic_cast<VariableExprAST*>(node)) {
        refs.push_back(v->getName());
        return;
    }
    if (dynamic_cast<NumberExprAST*>(node) || dynamic_cast<StringLiteralAST*>(node)) {
        return;
    }
    if (auto* assign = dynamic_cast<AssignmentExprAST*>(node)) {
        decls.push_back(assign->getVarName());
        collectVarRefsAndDecls(assign->getValue(), refs, decls);
        return;
    }
    if (auto* bin = dynamic_cast<BinaryExprAST*>(node)) {
        collectVarRefsAndDecls(bin->getLHS(), refs, decls);
        collectVarRefsAndDecls(bin->getRHS(), refs, decls);
        return;
    }
    if (auto* un = dynamic_cast<UnaryExprAST*>(node)) {
        collectVarRefsAndDecls(un->getOperand(), refs, decls);
        return;
    }
    if (auto* call = dynamic_cast<FunctionCallAST*>(node)) {
        collectVarRefsAndDecls(call->getCallee(), refs, decls);
        for (const auto& arg : call->getArgs())
            collectVarRefsAndDecls(arg.get(), refs, decls);
        return;
    }
    if (dynamic_cast<FunctionLiteralAST*>(node)) {
        return;  // nested fn literal — do not recurse into it
    }
    if (auto* block = dynamic_cast<BlockAST*>(node)) {
        for (const auto& stmt : block->getStatements())
            collectVarRefsAndDecls(stmt.get(), refs, decls);
        return;
    }
    if (auto* print = dynamic_cast<PrintStmtAST*>(node)) {
        collectVarRefsAndDecls(print->getFormatExpr(), refs, decls);
        for (const auto& arg : print->getArgs())
            collectVarRefsAndDecls(arg.get(), refs, decls);
        return;
    }
    if (auto* ret = dynamic_cast<ReturnStmtAST*>(node)) {
        if (ret->hasValue())
            collectVarRefsAndDecls(ret->getValue(), refs, decls);
        return;
    }
    if (auto* ifs = dynamic_cast<IfStmtAST*>(node)) {
        collectVarRefsAndDecls(ifs->getCondition(), refs, decls);
        collectVarRefsAndDecls(ifs->getThenStmt(), refs, decls);
        collectVarRefsAndDecls(ifs->getElseStmt(), refs, decls);
        return;
    }
    if (auto* wh = dynamic_cast<WhileStmtAST*>(node)) {
        collectVarRefsAndDecls(wh->getCondition(), refs, decls);
        collectVarRefsAndDecls(wh->getBody(), refs, decls);
        return;
    }
}

// Compute the ordered list of immutable free variables for a function body.
// Free vars: referenced in body, NOT in params, NOT in explicit mut() captures,
// NOT locally declared in body, AND exist in the current outer CodeGen scope.
static std::vector<std::string> computeFreeVars(
    ASTNode* body,
    const std::vector<FunctionParameter>& params,
    const std::vector<CapturedVariable>& captures,
    CodeGen& cg)
{
    std::vector<std::string> refs, decls;
    collectVarRefsAndDecls(body, refs, decls);

    std::set<std::string> excluded;
    for (const auto& p : params) excluded.insert(p.name);
    for (const auto& c : captures) excluded.insert(c.name);
    for (const auto& d : decls) excluded.insert(d);

    std::vector<std::string> result;
    std::set<std::string> seen;
    for (const auto& r : refs) {
        if (!excluded.count(r) && seen.insert(r).second && cg.getVariable(r)) {
            result.push_back(r);
        }
    }
    return result;
}

// AIDEV-NOTE: Closure bundle encoding:
// Each function value is a double encoding a ptr to a 2-element i64 bundle:
//   bundle[0] = LLVM function pointer (as i64)
//   bundle[1] = env pointer (as i64, 0 = no captures)
// The env is a heap-allocated array with N_free doubles followed by N_mut i64 ptr values.
//   env[0..N_free-1]       = immutable free var values (double)
//   env[N_free..N_free+N_mut-1] = mutable capture heap ptrs (i64/ptr, 8 bytes each)
// All generated LLVM functions take an extra 'ptr env' as their LAST parameter.
// At call site, the bundle is decoded to recover fn_ptr and env_ptr.
//
// Mutable captures: each mutable var gets a separate malloc(8) heap double shared
// between the outer scope (initialized once at closure creation) and the closure
// (reads/writes go through the heap ptr). State persists across closure calls.
// The sync stack in CodeGen tracks {local_alloca, shared_heap_ptr} pairs so that
// ReturnStmtAST::codegen() and block fallthrough can sync local state to heap before ret.
llvm::Value* FunctionLiteralAST::codegen() {
    auto& cg = getCodeGen();

    // Compute immutable free variables while still in the outer scope
    auto freeVars = computeFreeVars(body.get(), params, captures, cg);
    int N_free = static_cast<int>(freeVars.size());
    int N_mut  = static_cast<int>(captures.size());

    // Snapshot current values of immutable free variables from the outer scope
    std::vector<llvm::Value*> capturedValues;
    capturedValues.reserve(N_free);
    for (const auto& varName : freeVars) {
        auto* alloca = cg.getVariable(varName);
        auto* val = cg.getBuilder().CreateLoad(
            llvm::Type::getDoubleTy(cg.getContext()), alloca, varName + "_snap");
        capturedValues.push_back(val);
    }

    // Allocate heap doubles for mutable captures in the outer scope.
    // Each heap double is initialized with the current outer value and serves as
    // shared storage between the outer scope and all future closure calls.
    std::vector<llvm::Value*> mutCapturedPtrs;
    mutCapturedPtrs.reserve(N_mut);
    auto* mallocFn = getMalloc(cg);
    for (const auto& cap : captures) {
        auto* outerAlloca = cg.getVariable(cap.name);
        auto* outerVal = cg.getBuilder().CreateLoad(
            llvm::Type::getDoubleTy(cg.getContext()), outerAlloca, cap.name + "_outer");
        auto* heapSize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(cg.getContext()), 8);
        auto* heapMem = cg.getBuilder().CreateCall(mallocFn, {heapSize}, cap.name + "_mut_heap");
        cg.getBuilder().CreateStore(outerVal, heapMem);
        mutCapturedPtrs.push_back(heapMem);
    }

    // Save current insert point to restore after inner function body generation
    auto* savedBlock = cg.getBuilder().GetInsertBlock();

    // Build LLVM function type: double(double p1, ..., ptr env)
    std::vector<llvm::Type*> paramTypes(params.size(), llvm::Type::getDoubleTy(cg.getContext()));
    paramTypes.push_back(llvm::PointerType::getUnqual(cg.getContext()));  // env ptr
    auto* funcType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(cg.getContext()), paramTypes, /*isVarArg=*/false);

    std::string fnName = "__fn_" + std::to_string(fnCounter++);
    auto* func = llvm::Function::Create(
        funcType, llvm::Function::InternalLinkage, fnName, cg.getModule());

    // Name arguments
    {
        int i = 0;
        for (auto& arg : func->args()) {
            if (i < (int)params.size()) arg.setName(params[i++].name);
            else arg.setName("env");
        }
    }

    // Generate inner function body
    auto* entryBB = llvm::BasicBlock::Create(cg.getContext(), "entry", func);
    cg.getBuilder().SetInsertPoint(entryBB);

    cg.enterScope();

    // Declare user parameter allocas
    {
        int i = 0;
        for (auto& arg : func->args()) {
            if (i < (int)params.size()) {
                auto* alloca = cg.declareVariable(params[i].name, params[i].is_mutable, params[i].location);
                cg.getBuilder().CreateStore(&arg, alloca);
                ++i;
            }
        }
    }

    // Get envArg (last function argument) if there are any captures
    llvm::Value* envArg = nullptr;
    if (N_free > 0 || N_mut > 0) {
        int i = 0;
        for (auto& arg : func->args()) {
            if (i == (int)params.size()) { envArg = &arg; break; }
            ++i;
        }
    }

    // Load immutable captures from env and shadow outer scope vars with local allocas
    if (N_free > 0 && envArg) {
        for (int i = 0; i < N_free; ++i) {
            auto* elemPtr = cg.getBuilder().CreateConstGEP1_64(
                llvm::Type::getDoubleTy(cg.getContext()), envArg, i,
                freeVars[i] + "_env_ptr");
            auto* capVal = cg.getBuilder().CreateLoad(
                llvm::Type::getDoubleTy(cg.getContext()), elemPtr, freeVars[i] + "_cap");
            // Declare a local alloca with the same name, shadowing the outer scope
            auto* capAlloca = cg.declareVariable(freeVars[i], /*is_mutable=*/false, SourceLocation{});
            cg.getBuilder().CreateStore(capVal, capAlloca);
        }
    }

    // Load mutable capture heap ptrs from env, set up local mutable allocas,
    // and build the sync list for copy-back on return.
    std::vector<CodeGen::MutCaptureSync> syncList;
    if (N_mut > 0 && envArg) {
        syncList.reserve(N_mut);
        for (int j = 0; j < N_mut; ++j) {
            const auto& cap = captures[j];
            // env[N_free + j] stores the heap ptr as i64 (same 8-byte slot size as double)
            auto* i64Slot = cg.getBuilder().CreateConstGEP1_64(
                llvm::Type::getInt64Ty(cg.getContext()), envArg, N_free + j,
                cap.name + "_heap_slot");
            auto* heapPtrI64 = cg.getBuilder().CreateLoad(
                llvm::Type::getInt64Ty(cg.getContext()), i64Slot, cap.name + "_heap_i64");
            auto* heapPtr = cg.getBuilder().CreateIntToPtr(
                heapPtrI64, llvm::PointerType::getUnqual(cg.getContext()), cap.name + "_heap_ptr");
            // Load current value from heap into a local mutable alloca
            auto* curVal = cg.getBuilder().CreateLoad(
                llvm::Type::getDoubleTy(cg.getContext()), heapPtr, cap.name + "_cur");
            auto* localAlloca = cg.declareVariable(cap.name, /*is_mutable=*/true, cap.location);
            cg.getBuilder().CreateStore(curVal, localAlloca);
            syncList.push_back({localAlloca, heapPtr});
        }
    }
    // Always push (even empty) so ReturnStmtAST syncs the correct function's captures
    cg.pushMutCaptureSyncs(std::move(syncList));

    // Codegen the body
    llvm::Value* bodyVal = nullptr;
    if (is_expression_function) {
        auto* bodyExpr = dynamic_cast<ExprAST*>(body.get());
        if (bodyExpr) bodyVal = bodyExpr->codegen();
    } else {
        // Block-style: statements generate code; ReturnStmtAST emits ret directly
        body->codegen();
        if (!cg.getBuilder().GetInsertBlock()->getTerminator()) {
            // Fallthrough: sync mutable captures then emit implicit ret 0.0
            cg.emitMutCaptureSyncs();
            cg.getBuilder().CreateRet(
                llvm::ConstantFP::get(cg.getContext(), llvm::APFloat(0.0)));
        }
    }

    cg.exitScope();

    if (bodyVal) {
        // Expression function: sync mutable captures before emitting ret
        cg.emitMutCaptureSyncs();
        cg.getBuilder().CreateRet(bodyVal);
    }

    cg.popMutCaptureSyncs();

    // Restore the caller's insert point
    cg.getBuilder().SetInsertPoint(savedBlock);

    // --- Build closure bundle in the outer context ---

    // Allocate env array on the heap: N_free doubles + N_mut i64 ptr values (8 bytes each)
    int N_total = N_free + N_mut;
    llvm::Value* envPtrI64;
    if (N_total > 0) {
        auto* envSize = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(cg.getContext()),
            static_cast<uint64_t>(N_total * 8));
        auto* envMem = cg.getBuilder().CreateCall(mallocFn, {envSize}, "env_mem");

        // Store immutable captured doubles at env[0..N_free-1]
        for (int i = 0; i < N_free; ++i) {
            auto* elemPtr = cg.getBuilder().CreateConstGEP1_64(
                llvm::Type::getDoubleTy(cg.getContext()), envMem, i, "env_slot");
            cg.getBuilder().CreateStore(capturedValues[i], elemPtr);
        }

        // Store mutable capture heap ptrs (as i64) at env[N_free..N_free+N_mut-1]
        for (int j = 0; j < N_mut; ++j) {
            auto* ptrAsI64 = cg.getBuilder().CreatePtrToInt(
                mutCapturedPtrs[j], llvm::Type::getInt64Ty(cg.getContext()), "mut_ptr_i64");
            auto* slot = cg.getBuilder().CreateConstGEP1_64(
                llvm::Type::getInt64Ty(cg.getContext()), envMem, N_free + j, "mut_env_slot");
            cg.getBuilder().CreateStore(ptrAsI64, slot);
        }

        envPtrI64 = cg.getBuilder().CreatePtrToInt(
            envMem, llvm::Type::getInt64Ty(cg.getContext()), "env_ptr_i64");
    } else {
        envPtrI64 = llvm::ConstantInt::get(llvm::Type::getInt64Ty(cg.getContext()), 0);
    }

    // Malloc closure bundle: 2 x i64 = 16 bytes { fn_ptr_i64, env_ptr_i64 }
    auto* bundleMem = cg.getBuilder().CreateCall(
        mallocFn,
        {llvm::ConstantInt::get(llvm::Type::getInt64Ty(cg.getContext()), 16)},
        "closure_bundle");

    // Store fn_ptr as i64 at bundle[0]
    auto* fnPtrI64 = cg.getBuilder().CreatePtrToInt(
        func, llvm::Type::getInt64Ty(cg.getContext()), "fn_ptr_i64");
    cg.getBuilder().CreateStore(fnPtrI64, bundleMem);

    // Store env_ptr as i64 at bundle[1]
    auto* slot1 = cg.getBuilder().CreateConstGEP1_64(
        llvm::Type::getInt64Ty(cg.getContext()), bundleMem, 1, "bundle_slot1");
    cg.getBuilder().CreateStore(envPtrI64, slot1);

    // Encode bundle pointer as double: ptr -> i64 -> double
    auto* bundlePtrI64 = cg.getBuilder().CreatePtrToInt(
        bundleMem, llvm::Type::getInt64Ty(cg.getContext()), "bundle_i64");
    return cg.getBuilder().CreateBitCast(
        bundlePtrI64, llvm::Type::getDoubleTy(cg.getContext()), "bundle_double");
}

llvm::Value* FunctionCallAST::codegen() {
    auto& cg = getCodeGen();

    // Codegen the callee expression (closure bundle pointer encoded as double)
    llvm::Value* calleeVal = callee->codegen();
    if (!calleeVal) return nullptr;

    // Decode bundle pointer: double -> i64 -> ptr
    auto* bundlePtrI64 = cg.getBuilder().CreateBitCast(
        calleeVal, llvm::Type::getInt64Ty(cg.getContext()), "bundle_i64");
    auto* bundle = cg.getBuilder().CreateIntToPtr(
        bundlePtrI64, llvm::PointerType::getUnqual(cg.getContext()), "bundle_ptr");

    // Load fn_ptr_i64 from bundle[0], then convert to function pointer
    auto* fnPtrI64 = cg.getBuilder().CreateLoad(
        llvm::Type::getInt64Ty(cg.getContext()), bundle, "fn_ptr_i64");
    auto* fnPtr = cg.getBuilder().CreateIntToPtr(
        fnPtrI64, llvm::PointerType::getUnqual(cg.getContext()), "fn_ptr");

    // Load env_ptr_i64 from bundle[1], convert to ptr
    auto* slot1 = cg.getBuilder().CreateConstGEP1_64(
        llvm::Type::getInt64Ty(cg.getContext()), bundle, 1, "bundle_slot1");
    auto* envPtrI64 = cg.getBuilder().CreateLoad(
        llvm::Type::getInt64Ty(cg.getContext()), slot1, "env_ptr_i64");
    auto* envPtr = cg.getBuilder().CreateIntToPtr(
        envPtrI64, llvm::PointerType::getUnqual(cg.getContext()), "env_ptr");

    // Codegen user arguments
    std::vector<llvm::Value*> argValues;
    argValues.reserve(args.size() + 1);
    for (const auto& arg : args) {
        auto* v = arg->codegen();
        if (!v) return nullptr;
        argValues.push_back(v);
    }
    argValues.push_back(envPtr);  // env pointer is the last argument

    // Build function type: double(double*N, ptr)
    std::vector<llvm::Type*> paramTypes(args.size(), llvm::Type::getDoubleTy(cg.getContext()));
    paramTypes.push_back(llvm::PointerType::getUnqual(cg.getContext()));
    auto* funcType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(cg.getContext()), paramTypes, /*isVarArg=*/false);

    return cg.getBuilder().CreateCall(funcType, fnPtr, argValues, "calltmp");
}

llvm::Value* ReturnStmtAST::codegen() {
    auto& cg = getCodeGen();
    llvm::Value* retVal;
    if (value) {
        retVal = value->codegen();
        if (!retVal) return nullptr;
    } else {
        retVal = llvm::ConstantFP::get(cg.getContext(), llvm::APFloat(0.0));
    }
    // Sync mutable captures back to heap before returning
    cg.emitMutCaptureSyncs();
    cg.getBuilder().CreateRet(retVal);
    return retVal;
}
