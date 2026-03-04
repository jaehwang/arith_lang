#include "function_ast.h"
#include "codegen.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"

// Forward declaration (defined in codegen.cpp)
CodeGen& getCodeGen();

// Counter for generating unique function names
static int fnCounter = 0;

// AIDEV-NOTE: Function pointers are encoded as double values via ptrtoint+bitcast.
// This allows storing functions in the existing double-typed variable allocas.
// Encoding: ptr -> i64 (ptrtoint) -> double (bitcast)
// Decoding: double -> i64 (bitcast) -> ptr (inttoptr)
llvm::Value* FunctionLiteralAST::codegen() {
    auto& cg = getCodeGen();

    // Save current insert point so we can restore after generating the function body
    auto* savedBlock = cg.getBuilder().GetInsertBlock();

    // Build LLVM function type: N double params, returns double
    std::vector<llvm::Type*> paramTypes(params.size(), llvm::Type::getDoubleTy(cg.getContext()));
    auto* funcType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(cg.getContext()), paramTypes, /*isVarArg=*/false);

    // Create the LLVM function with a unique internal name
    std::string fnName = "__fn_" + std::to_string(fnCounter++);
    auto* func = llvm::Function::Create(
        funcType, llvm::Function::InternalLinkage, fnName, cg.getModule());

    // Name each LLVM argument to match the parameter names
    {
        int i = 0;
        for (auto& arg : func->args()) {
            arg.setName(params[i++].name);
        }
    }

    // Create function body and set insert point into the new function
    auto* entryBB = llvm::BasicBlock::Create(cg.getContext(), "entry", func);
    cg.getBuilder().SetInsertPoint(entryBB);

    // Enter a new scope and declare each parameter as a local alloca
    cg.enterScope();
    {
        int i = 0;
        for (auto& arg : func->args()) {
            auto* alloca = cg.declareVariable(params[i].name, params[i].is_mutable, params[i].location);
            cg.getBuilder().CreateStore(&arg, alloca);
            ++i;
        }
    }

    // Codegen the body
    llvm::Value* bodyVal = nullptr;
    if (is_expression_function) {
        auto* bodyExpr = dynamic_cast<ExprAST*>(body.get());
        if (bodyExpr) {
            bodyVal = bodyExpr->codegen();
        }
    } else {
        // Block-style: generate all statements; ReturnStmtAST emits ret directly
        body->codegen();
        // If control falls through without a return, add implicit ret 0.0
        if (!cg.getBuilder().GetInsertBlock()->getTerminator()) {
            cg.getBuilder().CreateRet(
                llvm::ConstantFP::get(cg.getContext(), llvm::APFloat(0.0)));
        }
    }

    cg.exitScope();

    if (bodyVal) {
        cg.getBuilder().CreateRet(bodyVal);
    }

    // Restore the caller's insert point
    cg.getBuilder().SetInsertPoint(savedBlock);

    // Encode function pointer as double: ptr -> i64 -> double
    auto* fnAsI64 = cg.getBuilder().CreatePtrToInt(
        func, llvm::Type::getInt64Ty(cg.getContext()), "fnptr_i64");
    return cg.getBuilder().CreateBitCast(
        fnAsI64, llvm::Type::getDoubleTy(cg.getContext()), "fnptr_double");
}

llvm::Value* FunctionCallAST::codegen() {
    auto& cg = getCodeGen();

    // Codegen the callee expression (function pointer encoded as double)
    llvm::Value* calleeVal = callee->codegen();
    if (!calleeVal) return nullptr;

    // Decode function pointer: double -> i64 -> ptr
    auto* fnAsI64 = cg.getBuilder().CreateBitCast(
        calleeVal, llvm::Type::getInt64Ty(cg.getContext()), "fnptr_i64");
    auto* fnPtr = cg.getBuilder().CreateIntToPtr(
        fnAsI64, llvm::PointerType::getUnqual(cg.getContext()), "fnptr");

    // Codegen all arguments
    std::vector<llvm::Value*> argValues;
    for (const auto& arg : args) {
        auto* v = arg->codegen();
        if (!v) return nullptr;
        argValues.push_back(v);
    }

    // Reconstruct the function type for the indirect call (all doubles)
    std::vector<llvm::Type*> paramTypes(args.size(), llvm::Type::getDoubleTy(cg.getContext()));
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
    cg.getBuilder().CreateRet(retVal);
    return retVal;
}
