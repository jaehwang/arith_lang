#include "codegen.h"
#include "ast.h"
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

llvm::Value* CodeGen::getVariable(const std::string& name) {
    auto it = namedValues.find(name);
    if (it != namedValues.end()) {
        return it->second;
    }
    return nullptr;
}

void CodeGen::setVariable(const std::string& name, llvm::Value* value) {
    namedValues[name] = value;
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
    llvm::Value* v = codeGenInstance->getVariable(name);
    if (!v) {
        throw std::runtime_error("Unknown variable name: " + name);
    }
    return v;
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
        default:
            throw std::runtime_error("Invalid binary operator");
    }
}

llvm::Value* AssignmentExprAST::codegen() {
    llvm::Value* val = value->codegen();
    if (!val) return nullptr;
    
    codeGenInstance->setVariable(varName, val);
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

void initializeCodeGen(const std::string& moduleName) {
    codeGenInstance = std::make_unique<CodeGen>(moduleName);
}

CodeGen& getCodeGen() {
    if (!codeGenInstance) {
        throw std::runtime_error("CodeGen not initialized");
    }
    return *codeGenInstance;
}