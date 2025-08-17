#include "parser.h"
#include <stdexcept>

Parser::Parser(Lexer& lexer) : lexer(lexer), currentToken(TOK_EOF), previousToken(TOK_EOF) {
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
    auto result = std::make_unique<NumberExprAST>(currentToken.numValue, currentToken.range.start);
    getNextToken();
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); // consume '('
    auto v = parseExpression();
    if (!v) return nullptr;
    
    if (currentToken.type != TOK_RPAREN)
        errorHere("Expected ')'");
    
    getNextToken(); // consume ')'
    return v;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = currentToken.value;
    SourceLocation idLoc = currentToken.range.start;
    getNextToken();
    return std::make_unique<VariableExprAST>(idName, idLoc);
}

std::unique_ptr<ExprAST> Parser::parseStringLiteral() {
    std::string strValue = currentToken.value;
    getNextToken();
    return std::make_unique<StringLiteralAST>(strValue, currentToken.range.start);
}

std::unique_ptr<ExprAST> Parser::parseUnaryExpr() {
    if (currentToken.type == TOK_MINUS) {
        char op = '-';
        SourceLocation opLoc = currentToken.range.start;
        getNextToken(); // consume the unary operator
        auto operand = parsePrimary();
        if (!operand) return nullptr;
        return std::make_unique<UnaryExprAST>(op, std::move(operand), opLoc);
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
            errorHere("Unknown token when expecting an expression");
    }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        int tokPrec = getTokenPrecedence();
        
        if (tokPrec < exprPrec)
            return lhs;
        
        int binOp = currentToken.type;
        SourceLocation opLoc = currentToken.range.start;
        getNextToken();
        
        auto rhs = parseUnaryExpr();
        if (!rhs) return nullptr;
        
        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec + 1, std::move(rhs));
            if (!rhs) return nullptr;
        }
        
        lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs), opLoc);
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
            // Point to the start of the LHS (previous token)
            errorAt("Invalid assignment target", previousToken.range.start);
        }
        
        std::string varName = varExpr->getName();
        // location of variable name is in previousToken (identifier) when '=' is current
        SourceLocation nameLoc = previousToken.range.start;
        getNextToken(); // consume '='
        
        auto rhs = parseExpression();
        if (!rhs) return nullptr;
        
        return std::make_unique<AssignmentExprAST>(varName, std::move(rhs), nameLoc);
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
        case TOK_MUT: {
            // Handle mutable variable declaration: mut x = value;
            getNextToken(); // consume 'mut'
            
            if (currentToken.type != TOK_IDENTIFIER) {
                errorHere("Expected variable name after 'mut'");
            }
            
            std::string varName = currentToken.value;
            SourceLocation nameLoc = currentToken.range.start;
            getNextToken(); // consume identifier
            
            if (currentToken.type != TOK_ASSIGN) {
                errorHere("Expected '=' after variable name in mutable declaration");
            }
            getNextToken(); // consume '='
            
            auto value = parseExpression();
            if (!value) return nullptr;
            
            // Semicolon is required
            if (currentToken.type != TOK_SEMICOLON) {
                errorAt("Expected ';' after mutable variable declaration", previousToken.range.end);
            }
            getNextToken(); // consume ';'
            
            return std::make_unique<AssignmentExprAST>(varName, std::move(value), true, AssignmentType::DECLARATION, nameLoc);
        }
        default: {
            auto expr = parseExpression();
            if (!expr) return nullptr;
            
            // Semicolon is required for expression statements
            if (currentToken.type != TOK_SEMICOLON) {
                // Point to where ';' should be: end of the previous token
                errorAt("Expected ';' after expression statement", previousToken.range.end);
            }
            getNextToken(); // consume ';'
            
            return expr;
        }
    }
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    SourceLocation printLoc = currentToken.range.start;
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
        // Point to where ';' should be: end of the previous token (end of last expr)
        errorAt("Expected ';' after print statement", previousToken.range.end);
    }
    getNextToken(); // consume ';'
    
    if (args.empty()) {
        return std::make_unique<PrintStmtAST>(std::move(firstExpr), printLoc);
    } else {
        return std::make_unique<PrintStmtAST>(std::move(firstExpr), std::move(args), printLoc);
    }
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    SourceLocation ifLoc = currentToken.range.start;
    getNextToken(); // consume 'if'
    
    if (currentToken.type != TOK_LPAREN) {
        errorHere("Expected '(' after 'if'");
    }
    getNextToken(); // consume '('
    
    auto condition = parseExpression();
    if (!condition) return nullptr;
    
    if (currentToken.type != TOK_RPAREN) {
        errorHere("Expected ')' after if condition");
    }
    getNextToken(); // consume ')'
    
    auto thenBlock = parseBlock();
    if (!thenBlock) return nullptr;
    
    std::unique_ptr<ASTNode> elseBlock = nullptr;
    if (currentToken.type == TOK_ELSE) {
        getNextToken(); // consume 'else'
    elseBlock = parseBlock();
        if (!elseBlock) return nullptr;
    } else {
    errorHere("Expected 'else' after 'if' statement");
    }
    return std::make_unique<IfStmtAST>(std::move(condition), 
                                       std::move(thenBlock), 
                                       std::move(elseBlock), ifLoc);
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    SourceLocation whileLoc = currentToken.range.start;
    getNextToken(); // consume 'while'
    
    if (currentToken.type != TOK_LPAREN) {
        errorHere("Expected '(' after 'while'");
    }
    getNextToken(); // consume '('
    
    auto condition = parseExpression();
    if (!condition) return nullptr;
    
    if (currentToken.type != TOK_RPAREN) {
        errorHere("Expected ')' after while condition");
    }
    getNextToken(); // consume ')'
    
    auto body = parseBlock();
    if (!body) return nullptr;
    
    return std::make_unique<WhileStmtAST>(std::move(condition), 
                                          std::move(body), whileLoc);
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    if (currentToken.type != TOK_LBRACE) {
        errorHere("Expected '{'");
    }
    getNextToken(); // consume '{'
    
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (currentToken.type != TOK_RBRACE && currentToken.type != TOK_EOF) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        } else {
            errorHere("Failed to parse statement in block");
        }
    }
    
    if (currentToken.type != TOK_RBRACE) {
        errorHere("Expected '}'");
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
        } catch (const ParseError&) {
            // Re-throw parse errors directly
            throw;
        } catch (const std::runtime_error& e) {
            // Wrap other runtime errors as parse errors at current token
            throw ParseError(std::string(e.what()), currentToken.range.start);
        }
    }
    
    return std::make_unique<ProgramAST>(std::move(statements));
}
