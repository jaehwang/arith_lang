#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "ast.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include <iostream>
#include <string>

void initializeCodeGen(const std::string& moduleName);
CodeGen& getCodeGen();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <expression>" << std::endl;
        return 1;
    }
    
    std::string input = argv[1];
    
    try {
        initializeCodeGen("ArithLang");
        
        auto& codeGen = getCodeGen();
        
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(codeGen.getContext()), false);
        llvm::Function* mainFunc = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, "main", codeGen.getModule());
        
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(
            codeGen.getContext(), "entry", mainFunc);
        codeGen.getBuilder().SetInsertPoint(entry);
        
        Lexer lexer(input);
        Parser parser(lexer);
        
        llvm::Value* lastResult = nullptr;
        bool hasStatements = false;
        
        // Parse all statements
        while (!lexer.isAtEnd()) {
            auto ast = parser.parseStatement();
            if (!ast) {
                break;
            }
            
            hasStatements = true;
            lastResult = ast->codegen();
            if (!lastResult) {
                std::cerr << "Failed to generate code" << std::endl;
                return 1;
            }
        }
        
        if (!hasStatements) {
            std::cerr << "No statements to parse" << std::endl;
            return 1;
        }
        
        // Always return 0.0 for statements (print statements return i32, but we need double)
        llvm::Value* zero = llvm::ConstantFP::get(codeGen.getContext(), llvm::APFloat(0.0));
        codeGen.getBuilder().CreateRet(zero);
        
        codeGen.printModule();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}