#pragma once
#include "lexer.h"
#include "ast.h"
#include <memory>
#include <map>
#include <stdexcept>

// ParseError exception carrying source location
class ParseError : public std::runtime_error {
public:
    SourceLocation loc;
    explicit ParseError(const std::string& message, SourceLocation where)
        : std::runtime_error(message), loc(std::move(where)) {}
};

class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    Token previousToken;
    std::map<int, int> binOpPrecedence;
    
    void getNextToken() { previousToken = currentToken; currentToken = lexer.getNextToken(); }
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseAssignment();
    std::unique_ptr<ASTNode> parsePrintStatement();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseUnaryExpr();
    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseNumberExpr();
    std::unique_ptr<ExprAST> parseIdentifierExpr();
    std::unique_ptr<ExprAST> parseStringLiteral();
    int getTokenPrecedence();
    std::unique_ptr<ASTNode> parseStatement();
    [[noreturn]] void errorHere(const std::string& msg) {
        throw ParseError(msg, currentToken.range.start);
    }
    [[noreturn]] void errorAt(const std::string& msg, SourceLocation loc) {
        throw ParseError(msg, std::move(loc));
    }
    
public:
    Parser(Lexer& lexer);
    std::unique_ptr<ASTNode> parseProgram();
};