#include "function_ast.h"

// AIDEV-TODO: US-009 - implement FunctionLiteralAST codegen for expression-body functions
llvm::Value* FunctionLiteralAST::codegen() {
    return nullptr;
}

// AIDEV-TODO: US-009 - implement FunctionCallAST codegen (llvm::CallInst)
llvm::Value* FunctionCallAST::codegen() {
    return nullptr;
}

// AIDEV-TODO: US-010 - implement ReturnStmtAST codegen (llvm::ReturnInst)
llvm::Value* ReturnStmtAST::codegen() {
    return nullptr;
}
