#pragma once
#include <memory>
#include <vector>
#include <string>
#include "lexer.h"

namespace llvm {
    class Value;
}

// Variable assignment type classification
enum class AssignmentType {
    DECLARATION,     // First binding: x = 42 or mut x = 42
    REASSIGNMENT,    // Subsequent assignment: x = 43 (only valid for mutable)
    SHADOWING        // New immutable binding with same name
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen() = 0;
};

class ExprAST : public ASTNode {
public:
    virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST {
    double val;
public:
    NumberExprAST(double val) : val(val) {}
    llvm::Value* codegen() override;
    double getValue() const { return val; }
};

class VariableExprAST : public ExprAST {
    std::string name;
    SourceLocation name_location{}; // location of identifier for diagnostics
public:
    VariableExprAST(const std::string& name) : name(name) {}
    VariableExprAST(const std::string& name, SourceLocation loc) : name(name), name_location(std::move(loc)) {}
    llvm::Value* codegen() override;
    const std::string& getName() const { return name; }
    const SourceLocation& getNameLocation() const { return name_location; }
};

class StringLiteralAST : public ExprAST {
    std::string value;
public:
    StringLiteralAST(const std::string& value) : value(value) {}
    llvm::Value* codegen() override;
    const std::string& getValue() const { return value; }
};

class UnaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> operand;
public:
    UnaryExprAST(char op, std::unique_ptr<ExprAST> operand)
        : op(op), operand(std::move(operand)) {}
    llvm::Value* codegen() override;
    char getOperator() const { return op; }
    ExprAST* getOperand() const { return operand.get(); }
};

class BinaryExprAST : public ExprAST {
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    llvm::Value* codegen() override;
    char getOperator() const { return op; }
    ExprAST* getLHS() const { return lhs.get(); }
    ExprAST* getRHS() const { return rhs.get(); }
};

class AssignmentExprAST : public ExprAST {
    std::string varName;
    std::unique_ptr<ExprAST> value;
    bool is_mutable_declaration;
    AssignmentType assignment_type;
    SourceLocation name_location; // location of variable name for diagnostics
public:
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value)
        : varName(varName), value(std::move(value)), is_mutable_declaration(false), assignment_type(AssignmentType::DECLARATION), name_location{} {}
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value,
                      SourceLocation loc)
        : varName(varName), value(std::move(value)), is_mutable_declaration(false), assignment_type(AssignmentType::DECLARATION), name_location(std::move(loc)) {}
    
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value, 
                      bool is_mut, AssignmentType type)
    : varName(varName), value(std::move(value)), is_mutable_declaration(is_mut), assignment_type(type), name_location{} {}
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value, 
                      bool is_mut, AssignmentType type, SourceLocation loc)
        : varName(varName), value(std::move(value)), is_mutable_declaration(is_mut), assignment_type(type), name_location(std::move(loc)) {}
    
    llvm::Value* codegen() override;
    const std::string& getVarName() const { return varName; }
    ExprAST* getValue() const { return value.get(); }
    bool isMutableDeclaration() const { return is_mutable_declaration; }
    AssignmentType getAssignmentType() const { return assignment_type; }
    const SourceLocation& getNameLocation() const { return name_location; }
};

class PrintStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> formatExpr;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    PrintStmtAST(std::unique_ptr<ExprAST> formatExpr) : formatExpr(std::move(formatExpr)) {}
    PrintStmtAST(std::unique_ptr<ExprAST> formatExpr, std::vector<std::unique_ptr<ExprAST>> args) 
        : formatExpr(std::move(formatExpr)), args(std::move(args)) {}
    llvm::Value* codegen() override;
    ExprAST* getFormatExpr() const { return formatExpr.get(); }
    const std::vector<std::unique_ptr<ExprAST>>& getArgs() const { return args; }
    
    // Backward compatibility
    ExprAST* getExpr() const { return formatExpr.get(); }
};

class IfStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ASTNode> thenStmt;
    std::unique_ptr<ASTNode> elseStmt;
public:
    IfStmtAST(std::unique_ptr<ExprAST> condition, 
              std::unique_ptr<ASTNode> thenStmt,
              std::unique_ptr<ASTNode> elseStmt)
        : condition(std::move(condition)), 
          thenStmt(std::move(thenStmt)), 
          elseStmt(std::move(elseStmt)) {}
    llvm::Value* codegen() override;
    ExprAST* getCondition() const { return condition.get(); }
    ASTNode* getThenStmt() const { return thenStmt.get(); }
    ASTNode* getElseStmt() const { return elseStmt.get(); }
};

class WhileStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ASTNode> body;
public:
    WhileStmtAST(std::unique_ptr<ExprAST> condition, 
                 std::unique_ptr<ASTNode> body)
        : condition(std::move(condition)), 
          body(std::move(body)) {}
    llvm::Value* codegen() override;
    ExprAST* getCondition() const { return condition.get(); }
    ASTNode* getBody() const { return body.get(); }
};

class BlockAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    BlockAST(std::vector<std::unique_ptr<ASTNode>> statements)
        : statements(std::move(statements)) {}
    llvm::Value* codegen() override;
    const std::vector<std::unique_ptr<ASTNode>>& getStatements() const { return statements; }
};

class ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    ProgramAST(std::vector<std::unique_ptr<ASTNode>> statements)
        : statements(std::move(statements)) {}
    llvm::Value* codegen() override;
    const std::vector<std::unique_ptr<ASTNode>>& getStatements() const { return statements; }
};
