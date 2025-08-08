#include "parser.h"
#include <stdexcept>

Parser::Parser(Lexer& lexer) : lexer(lexer), currentToken(TOK_EOF) {
    // Comparison operators (lowest precedence)
    binOpPrecedence[TOK_EQ] = 5;
    binOpPrecedence[TOK_NEQ] = 5;
    binOpPrecedence[TOK_LT] = 5;
    binOpPrecedence[TOK_LTE] = 5;
    binOpPrecedence[TOK_GT] = 5;
    binOpPrecedence[TOK_GTE] = 5;
    
    // Arithmetic operators
    binOpPrecedence['+'] = 10;
    binOpPrecedence['-'] = 10;
    binOpPrecedence['*'] = 40;
    binOpPrecedence['/'] = 40;
    
    getNextToken();
}

int Parser::getTokenPrecedence() {
    auto it = binOpPrecedence.find(currentToken.type);
    if (it == binOpPrecedence.end()) return -1;
    
    int tokPrec = it->second;
    if (tokPrec <= 0) return -1;
    return tokPrec;
}

std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(currentToken.numValue);
    getNextToken();
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); // consume '('
    auto v = parseExpression();
    if (!v) return nullptr;
    
    if (currentToken.type != TOK_RPAREN)
        throw std::runtime_error("Expected ')'");
    
    getNextToken(); // consume ')'
    return v;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = currentToken.value;
    getNextToken();
    return std::make_unique<VariableExprAST>(idName);
}

std::unique_ptr<ExprAST> Parser::parseStringLiteral() {
    std::string strValue = currentToken.value;
    getNextToken();
    return std::make_unique<StringLiteralAST>(strValue);
}

std::unique_ptr<ExprAST> Parser::parseUnaryExpr() {
    if (currentToken.type == TOK_MINUS) {
        char op = '-';
        getNextToken(); // consume the unary operator
        auto operand = parsePrimary();
        if (!operand) return nullptr;
        return std::make_unique<UnaryExprAST>(op, std::move(operand));
    }
    
    return parsePrimary();
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken.type) {
        case TOK_IDENTIFIER:
            return parseIdentifierExpr();
        case TOK_NUMBER:
            return parseNumberExpr();
        case TOK_STRING:
            return parseStringLiteral();
        case TOK_LPAREN:
            return parseParenExpr();
        case TOK_MINUS:
            return parseUnaryExpr();
        default:
            throw std::runtime_error("Unknown token when expecting an expression");
    }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int tokPrec = getTokenPrecedence();
        
        if (tokPrec < exprPrec)
            return lhs;
        
        int binOp = currentToken.type;
        getNextToken();
        
        auto rhs = parseUnaryExpr();
        if (!rhs) return nullptr;
        
        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
            if (!rhs) return nullptr;
        }
        
        lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<ExprAST> Parser::parseAssignment() {
    auto lhs = parseUnaryExpr();
    if (!lhs) return nullptr;
    
    // Check if this is an assignment
    if (currentToken.type == TOK_ASSIGN) {
        // lhs must be a variable for assignment
        auto* varExpr = dynamic_cast<VariableExprAST*>(lhs.get());
        if (!varExpr) {
            throw std::runtime_error("Invalid assignment target");
        }
        
        std::string varName = varExpr->getName();
        getNextToken(); // consume '='
        
        auto rhs = parseExpression();
        if (!rhs) return nullptr;
        
        return std::make_unique<AssignmentExprAST>(varName, std::move(rhs));
    }
    
    // Not an assignment, parse as regular expression
    return parseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    switch (currentToken.type) {
        case TOK_PRINT:
            return parsePrintStatement();
        case TOK_IF:
            return parseIfStatement();
        case TOK_WHILE:
            return parseWhileStatement();
        default: {
            auto expr = parseExpression();
            if (!expr) return nullptr;
            
            // Semicolon is required for expression statements
            if (currentToken.type != TOK_SEMICOLON) {
                throw std::runtime_error("Expected ';' after expression statement");
            }
            getNextToken(); // consume ';'
            
            return expr;
        }
    }
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    getNextToken(); // consume 'print'
    
    auto firstExpr = parseExpression();
    if (!firstExpr) return nullptr;
    
    // Check if there are additional arguments (comma-separated)
    std::vector<std::unique_ptr<ExprAST>> args;
    while (currentToken.type == TOK_COMMA) {
        getNextToken(); // consume ','
        auto arg = parseExpression();
        if (!arg) return nullptr;
        args.push_back(std::move(arg));
    }
    
    if (currentToken.type != TOK_SEMICOLON) {
        throw std::runtime_error("Expected ';' after print statement");
    }
    getNextToken(); // consume ';'
    
    if (args.empty()) {
        return std::make_unique<PrintStmtAST>(std::move(firstExpr));
    } else {
        return std::make_unique<PrintStmtAST>(std::move(firstExpr), std::move(args));
    }
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    getNextToken(); // consume 'if'
    
    if (currentToken.type != TOK_LPAREN) {
        throw std::runtime_error("Expected '(' after 'if'");
    }
    getNextToken(); // consume '('
    
    auto condition = parseExpression();
    if (!condition) return nullptr;
    
    if (currentToken.type != TOK_RPAREN) {
        throw std::runtime_error("Expected ')' after if condition");
    }
    getNextToken(); // consume ')'
    
    auto thenBlock = parseBlock();
    if (!thenBlock) return nullptr;
    
    std::unique_ptr<ASTNode> elseBlock = nullptr;
    if (currentToken.type == TOK_ELSE) {
        getNextToken(); // consume 'else'
        elseBlock = parseBlock();
        if (!elseBlock) return nullptr;
    }
    
    return std::make_unique<IfStmtAST>(std::move(condition), 
                                       std::move(thenBlock), 
                                       std::move(elseBlock));
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    getNextToken(); // consume 'while'
    
    if (currentToken.type != TOK_LPAREN) {
        throw std::runtime_error("Expected '(' after 'while'");
    }
    getNextToken(); // consume '('
    
    auto condition = parseExpression();
    if (!condition) return nullptr;
    
    if (currentToken.type != TOK_RPAREN) {
        throw std::runtime_error("Expected ')' after while condition");
    }
    getNextToken(); // consume ')'
    
    auto body = parseBlock();
    if (!body) return nullptr;
    
    return std::make_unique<WhileStmtAST>(std::move(condition), 
                                          std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    if (currentToken.type != TOK_LBRACE) {
        throw std::runtime_error("Expected '{'");
    }
    getNextToken(); // consume '{'
    
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (currentToken.type != TOK_RBRACE && currentToken.type != TOK_EOF) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        } else {
            throw std::runtime_error("Failed to parse statement in block");
        }
    }
    
    if (currentToken.type != TOK_RBRACE) {
        throw std::runtime_error("Expected '}'");
    }
    getNextToken(); // consume '}'
    
    return std::make_unique<BlockAST>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseProgram() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (currentToken.type != TOK_EOF) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            } else {
                // Statement parsing failed - this should not happen with proper error handling
                throw std::runtime_error("Failed to parse statement");
            }
        } catch (const std::runtime_error& e) {
            // If any statement fails to parse, the entire program is invalid
            throw std::runtime_error("Parse error: " + std::string(e.what()));
        }
    }
    
    return std::make_unique<ProgramAST>(std::move(statements));
}