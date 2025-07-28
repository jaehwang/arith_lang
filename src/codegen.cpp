#include "codegen.h"
#include "ast.h"
#include "lexer.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include <stdexcept>
#include <vector>

static std::unique_ptr<CodeGen> codeGenInstance;

CodeGen::CodeGen(const std::string& moduleName) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

llvm::AllocaInst* CodeGen::createVariable(const std::string& name) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst* alloca = tmpB.CreateAlloca(llvm::Type::getDoubleTy(*context), nullptr, name);
    namedValues[name] = alloca;
    return alloca;
}

llvm::AllocaInst* CodeGen::getVariable(const std::string& name) {
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        return it->second;
    }
    return nullptr;
}

void CodeGen::setVariable(const std::string& name, llvm::AllocaInst* alloca) {
    namedValues[name] = alloca;
}

llvm::Function* CodeGen::getPrintfDeclaration() {
    llvm::Function* printfFunc = module->getFunction("printf");
    if (!printfFunc) {
        llvm::FunctionType* printfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*context),
            {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*context))}, 
            true
        );
        printfFunc = llvm::Function::Create(
            printfType, 
            llvm::Function::ExternalLinkage, 
            "printf", 
            *module
        );
    }
    return printfFunc;
}

void CodeGen::printModule() {
    module->print(llvm::outs(), nullptr);
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(val));
}

llvm::Value* VariableExprAST::codegen() {
    llvm::AllocaInst* alloca = codeGenInstance->getVariable(name);
    if (!alloca) {
        throw std::runtime_error("Unknown variable name: " + name);
    }
    return codeGenInstance->getBuilder().CreateLoad(llvm::Type::getDoubleTy(codeGenInstance->getContext()), alloca, name);
}

llvm::Value* BinaryExprAST::codegen() {
    llvm::Value* l = lhs->codegen();
    llvm::Value* r = rhs->codegen();
    if (!l || !r) return nullptr;
    
    switch (op) {
        case '+':
            return codeGenInstance->getBuilder().CreateFAdd(l, r, "addtmp");
        case '-':
            return codeGenInstance->getBuilder().CreateFSub(l, r, "subtmp");
        case '*':
            return codeGenInstance->getBuilder().CreateFMul(l, r, "multmp");
        case '/':
            return codeGenInstance->getBuilder().CreateFDiv(l, r, "divtmp");
        case TOK_LT: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOLT(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_LTE: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOLE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_GT: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOGT(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_GTE: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOGE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_EQ: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOEQ(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_NEQ: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpONE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        default:
            throw std::runtime_error("Invalid binary operator");
    }
}

llvm::Value* AssignmentExprAST::codegen() {
    llvm::Value* val = value->codegen();
    if (!val) return nullptr;
    
    llvm::AllocaInst* alloca = codeGenInstance->getVariable(varName);
    if (!alloca) {
        alloca = codeGenInstance->createVariable(varName);
    }
    
    codeGenInstance->getBuilder().CreateStore(val, alloca);
    return val;
}

llvm::Value* PrintStmtAST::codegen() {
    llvm::Value* val = expr->codegen();
    if (!val) return nullptr;
    
    // Create format string "%f\n"
    llvm::Constant* formatStr = codeGenInstance->getBuilder().CreateGlobalString("%f\n");
    
    // Get printf function
    llvm::Function* printfFunc = codeGenInstance->getPrintfDeclaration();
    
    // Call printf with format string and value
    std::vector<llvm::Value*> args = {formatStr, val};
    return codeGenInstance->getBuilder().CreateCall(printfFunc, args, "printfcall");
}

llvm::Value* IfStmtAST::codegen() {
    llvm::Value* condV = condition->codegen();
    if (!condV) return nullptr;
    
    // Convert condition to boolean by comparing with 0.0
    condV = codeGenInstance->getBuilder().CreateFCmpONE(
        condV, 
        llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0)), 
        "ifcond"
    );
    
    llvm::Function* function = codeGenInstance->getBuilder().GetInsertBlock()->getParent();
    
    // Create blocks for then, else, and merge
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "ifcont");
    
    codeGenInstance->getBuilder().CreateCondBr(condV, thenBB, elseBB);
    
    // Emit then block
    codeGenInstance->getBuilder().SetInsertPoint(thenBB);
    llvm::Value* thenV = thenStmt->codegen();
    if (!thenV) return nullptr;
    
    // Convert to double if needed
    if (thenV->getType()->isIntegerTy()) {
        thenV = codeGenInstance->getBuilder().CreateSIToFP(
            thenV, 
            llvm::Type::getDoubleTy(codeGenInstance->getContext()), 
            "int2double"
        );
    }
    
    codeGenInstance->getBuilder().CreateBr(mergeBB);
    thenBB = codeGenInstance->getBuilder().GetInsertBlock();
    
    // Emit else block
    function->insert(function->end(), elseBB);
    codeGenInstance->getBuilder().SetInsertPoint(elseBB);
    
    llvm::Value* elseV = nullptr;
    if (elseStmt) {
        elseV = elseStmt->codegen();
        if (!elseV) return nullptr;
        
        // Convert to double if needed
        if (elseV->getType()->isIntegerTy()) {
            elseV = codeGenInstance->getBuilder().CreateSIToFP(
                elseV, 
                llvm::Type::getDoubleTy(codeGenInstance->getContext()), 
                "int2double"
            );
        }
    } else {
        elseV = llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
    }
    
    codeGenInstance->getBuilder().CreateBr(mergeBB);
    elseBB = codeGenInstance->getBuilder().GetInsertBlock();
    
    // Emit merge block
    function->insert(function->end(), mergeBB);
    codeGenInstance->getBuilder().SetInsertPoint(mergeBB);
    
    llvm::PHINode* pn = codeGenInstance->getBuilder().CreatePHI(
        llvm::Type::getDoubleTy(codeGenInstance->getContext()), 2, "iftmp"
    );
    pn->addIncoming(thenV, thenBB);
    pn->addIncoming(elseV, elseBB);
    
    return pn;
}

llvm::Value* WhileStmtAST::codegen() {
    llvm::Function* function = codeGenInstance->getBuilder().GetInsertBlock()->getParent();
    
    // Create blocks for loop condition, body, and after loop
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "loopcond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "loop");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "afterloop");
    
    // Jump to condition block
    codeGenInstance->getBuilder().CreateBr(condBB);
    
    // Emit condition block
    codeGenInstance->getBuilder().SetInsertPoint(condBB);
    llvm::Value* condV = condition->codegen();
    if (!condV) return nullptr;
    
    // Convert condition to boolean by comparing with 0.0
    condV = codeGenInstance->getBuilder().CreateFCmpONE(
        condV, 
        llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0)), 
        "loopcond"
    );
    
    codeGenInstance->getBuilder().CreateCondBr(condV, loopBB, afterBB);
    
    // Emit loop body
    function->insert(function->end(), loopBB);
    codeGenInstance->getBuilder().SetInsertPoint(loopBB);
    
    llvm::Value* bodyV = body->codegen();
    if (!bodyV) return nullptr;
    
    // Jump back to condition
    codeGenInstance->getBuilder().CreateBr(condBB);
    
    // Emit after loop block
    function->insert(function->end(), afterBB);
    codeGenInstance->getBuilder().SetInsertPoint(afterBB);
    
    // Return a constant value for while loops
    return llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
}

llvm::Value* BlockAST::codegen() {
    llvm::Value* lastValue = nullptr;
    
    for (const auto& stmt : statements) {
        lastValue = stmt->codegen();
        if (!lastValue) return nullptr;
    }
    
    // Return the last statement's value, or 0.0 if no statements
    return lastValue ? lastValue : llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
}

void initializeCodeGen(const std::string& moduleName) {
    codeGenInstance = std::make_unique<CodeGen>(moduleName);
}

CodeGen& getCodeGen() {
    if (!codeGenInstance) {
        throw std::runtime_error("CodeGen not initialized");
    }
    return *codeGenInstance;
}