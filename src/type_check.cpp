#include "type_check.h"
#include <stdexcept>

void typeCheck(ASTNode* node) {
    if (!node) return;

    // Binary Operation
    if (auto bin = dynamic_cast<BinaryExprAST*>(node)) {
        typeCheck(bin->getLHS());
        typeCheck(bin->getRHS());
        if (dynamic_cast<StringLiteralAST*>(bin->getLHS())) {
            throw std::runtime_error("String literal cannot be used in binary operation (left operand)");
        }
        if (dynamic_cast<StringLiteralAST*>(bin->getRHS())) {
            throw std::runtime_error("String literal cannot be used in binary operation (right operand)");
        }
        return;
    }

    // Unary Operation
    if (auto unary = dynamic_cast<UnaryExprAST*>(node)) {
        typeCheck(unary->getOperand());
        if (dynamic_cast<StringLiteralAST*>(unary->getOperand())) {
            throw std::runtime_error("String literal cannot be used in unary operation");
        }
        return;
    }

    // Assignment
    if (auto assign = dynamic_cast<AssignmentExprAST*>(node)) {
        typeCheck(assign->getValue());
        return;
    }

    // Print Statement
    if (auto print = dynamic_cast<PrintStmtAST*>(node)) {
        typeCheck(print->getFormatExpr());
        for (const auto& arg : print->getArgs()) {
            typeCheck(arg.get());
        }
        return;
    }

    // If Statement
    if (auto ifstmt = dynamic_cast<IfStmtAST*>(node)) {
        typeCheck(ifstmt->getCondition());
        typeCheck(ifstmt->getThenStmt());
        typeCheck(ifstmt->getElseStmt());
        return;
    }

    // While Statement
    if (auto whilestmt = dynamic_cast<WhileStmtAST*>(node)) {
        typeCheck(whilestmt->getCondition());
        typeCheck(whilestmt->getBody());
        return;
    }

    // Block
    if (auto block = dynamic_cast<BlockAST*>(node)) {
        for (const auto& stmt : block->getStatements()) {
            typeCheck(stmt.get());
        }
        return;
    }

    // Program
    if (auto program = dynamic_cast<ProgramAST*>(node)) {
        for (const auto& stmt : program->getStatements()) {
            typeCheck(stmt.get());
        }
        return;
    }

    // Number, Variable, StringLiteral 등은 별도 체크 불필요
}
