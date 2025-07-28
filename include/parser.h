#pragma once
#include "lexer.h"
#include "ast.h"
#include <memory>
#include <map>

class Parser {
private:
    Lexer& lexer;
    Token currentToken;
    std::map<int, int> binOpPrecedence;
    
    void getNextToken() { currentToken = lexer.getNextToken(); }
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseAssignment();
    std::unique_ptr<ASTNode> parsePrintStatement();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseWhileStatement();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseNumberExpr();
    std::unique_ptr<ExprAST> parseIdentifierExpr();
    int getTokenPrecedence();
    
public:
    Parser(Lexer& lexer);
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ExprAST> parse();
};