#pragma once
#include <memory>
#include <vector>

namespace llvm {
    class Value;
}

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
public:
    VariableExprAST(const std::string& name) : name(name) {}
    llvm::Value* codegen() override;
    const std::string& getName() const { return name; }
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
public:
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value)
        : varName(varName), value(std::move(value)) {}
    llvm::Value* codegen() override;
    const std::string& getVarName() const { return varName; }
    ExprAST* getValue() const { return value.get(); }
};

class PrintStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> expr;
public:
    PrintStmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)) {}
    llvm::Value* codegen() override;
    ExprAST* getExpr() const { return expr.get(); }
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