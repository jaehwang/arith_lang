#pragma once
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>
#include <memory>

class CodeGen {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::map<std::string, llvm::AllocaInst*> namedValues;
    
public:
    CodeGen(const std::string& moduleName);
    
    llvm::LLVMContext& getContext() { return *context; }
    llvm::Module& getModule() { return *module; }
    llvm::IRBuilder<>& getBuilder() { return *builder; }
    
    llvm::AllocaInst* createVariable(const std::string& name);
    llvm::AllocaInst* getVariable(const std::string& name);
    void setVariable(const std::string& name, llvm::AllocaInst* alloca);
    
    llvm::Function* getPrintfDeclaration();
    
    void printModule();
    void writeObjectFile(const std::string& filename);
};