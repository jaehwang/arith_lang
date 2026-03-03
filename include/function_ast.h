#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "ast.h"

// Forward declarations for LLVM types
namespace llvm {
    class Function;
}

// FunctionParameter: a named parameter with optional mutability
struct FunctionParameter {
    std::string name;
    bool is_mutable;
    SourceLocation location;
};

// CapturedVariable: a variable explicitly captured by a closure
// AIDEV-NOTE: only mutable captures use mut() clause; immutable captures are auto-detected at codegen
struct CapturedVariable {
    std::string name;
    bool is_mutable_capture;  // true if declared via mut(var) clause
    SourceLocation location;
};

// ClosureContext: runtime capture storage for a closure instance
// AIDEV-NOTE: immutable captures store by-value doubles; mutable captures share heap via shared_ptr
struct ClosureContext {
    std::map<std::string, double> immutable_captures;
    std::map<std::string, std::shared_ptr<double>> mutable_captures;
};

// FunctionValue: runtime representation of a first-class function
struct FunctionValue {
    llvm::Function* llvm_function = nullptr;
    ClosureContext closure_context;
};

// FunctionLiteralAST: fn(params) => expr  OR  fn(params) mut(vars) { stmts }
class FunctionLiteralAST : public ExprAST {
    std::vector<FunctionParameter> params;
    std::vector<CapturedVariable> captures;
    std::unique_ptr<ASTNode> body;
    bool is_expression_function;  // true for => form, false for { block } form
    SourceLocation fn_location;
public:
    FunctionLiteralAST(std::vector<FunctionParameter> params,
                       std::vector<CapturedVariable> captures,
                       std::unique_ptr<ASTNode> body,
                       bool is_expression_function,
                       SourceLocation loc)
        : params(std::move(params)), captures(std::move(captures)),
          body(std::move(body)), is_expression_function(is_expression_function),
          fn_location(std::move(loc)) {}

    llvm::Value* codegen() override;

    const std::vector<FunctionParameter>& getParams() const { return params; }
    const std::vector<CapturedVariable>& getCaptures() const { return captures; }
    ASTNode* getBody() const { return body.get(); }
    bool isExpressionFunction() const { return is_expression_function; }
    const SourceLocation& getFnLocation() const { return fn_location; }
};

// FunctionCallAST: callee(arg1, arg2, ...)
class FunctionCallAST : public ExprAST {
    std::unique_ptr<ExprAST> callee;
    std::vector<std::unique_ptr<ExprAST>> args;
    SourceLocation call_location;
public:
    FunctionCallAST(std::unique_ptr<ExprAST> callee,
                    std::vector<std::unique_ptr<ExprAST>> args,
                    SourceLocation loc)
        : callee(std::move(callee)), args(std::move(args)),
          call_location(std::move(loc)) {}

    llvm::Value* codegen() override;

    ExprAST* getCallee() const { return callee.get(); }
    const std::vector<std::unique_ptr<ExprAST>>& getArgs() const { return args; }
    const SourceLocation& getCallLocation() const { return call_location; }
};

// ReturnStmtAST: return; or return expr;
// AIDEV-NOTE: value is nullptr for bare return (void); checked at type-check in US-008
class ReturnStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> value;  // nullptr for bare return
    SourceLocation return_location;
public:
    ReturnStmtAST(std::unique_ptr<ExprAST> value, SourceLocation loc)
        : value(std::move(value)), return_location(std::move(loc)) {}

    llvm::Value* codegen() override;

    ExprAST* getValue() const { return value.get(); }
    bool hasValue() const { return value != nullptr; }
    const SourceLocation& getReturnLocation() const { return return_location; }
};
