#include "parser.h"
#include "function_ast.h"
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

std::unique_ptr<ExprAST> Parser::parsePostfixExpr() {
    auto expr = parsePrimary();
    if (!expr) return nullptr;

    while (currentToken.type == TOK_LPAREN) {
        expr = parseFunctionCall(std::move(expr));
        if (!expr) return nullptr;
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseFunctionCall(std::unique_ptr<ExprAST> callee) {
    SourceLocation callLoc = currentToken.range.start; // location of '('
    getNextToken(); // consume '('

    std::vector<std::unique_ptr<ExprAST>> args;
    if (currentToken.type != TOK_RPAREN) {
        auto arg = parseExpression();
        if (!arg) return nullptr;
        args.push_back(std::move(arg));

        while (currentToken.type == TOK_COMMA) {
            getNextToken(); // consume ','
            auto arg = parseExpression();
            if (!arg) return nullptr;
            args.push_back(std::move(arg));
        }
    }

    if (currentToken.type != TOK_RPAREN)
        errorHere("Expected ')' after function call arguments");
    getNextToken(); // consume ')'

    return std::make_unique<FunctionCallAST>(std::move(callee), std::move(args), callLoc);
}

std::unique_ptr<ExprAST> Parser::parseUnaryExpr() {
    if (currentToken.type == TOK_MINUS) {
        char op = '-';
        SourceLocation opLoc = currentToken.range.start;
        getNextToken(); // consume the unary operator
        auto operand = parsePostfixExpr();
        if (!operand) return nullptr;
        return std::make_unique<UnaryExprAST>(op, std::move(operand), opLoc);
    }

    return parsePostfixExpr();
}

std::unique_ptr<ExprAST> Parser::parseFunctionLiteral() {
    SourceLocation fnLoc = currentToken.range.start;
    getNextToken(); // consume 'fn'

    if (currentToken.type != TOK_LPAREN)
        errorHere("Expected '(' after 'fn'");
    getNextToken(); // consume '('

    // Parse a single parameter: [mut] identifier
    auto parseOneParam = [&]() -> FunctionParameter {
        bool is_mutable = false;
        if (currentToken.type == TOK_MUT) {
            is_mutable = true;
            getNextToken(); // consume 'mut'
        }
        if (currentToken.type != TOK_IDENTIFIER)
            errorHere("Expected parameter name in function parameter list");
        FunctionParameter p{currentToken.value, is_mutable, currentToken.range.start};
        getNextToken(); // consume identifier
        return p;
    };

    std::vector<FunctionParameter> params;
    if (currentToken.type != TOK_RPAREN) {
        params.push_back(parseOneParam());

        while (currentToken.type == TOK_COMMA) {
            getNextToken(); // consume ','

            if (currentToken.type == TOK_RPAREN)
                errorHere("Trailing comma in parameter list");

            params.push_back(parseOneParam());
        }
    }

    if (currentToken.type != TOK_RPAREN)
        errorHere("Expected ')' after function parameters");
    getNextToken(); // consume ')'

    // Parse optional capture clause: mut(var1, var2, ...)
    std::vector<CapturedVariable> captures;
    if (currentToken.type == TOK_MUT)
        captures = parseCaptureClause();

    if (currentToken.type == TOK_ARROW) {
        getNextToken(); // consume '=>'

        auto bodyExpr = parseExpression();
        if (!bodyExpr) return nullptr;

        return std::make_unique<FunctionLiteralAST>(
            std::move(params),
            std::move(captures),
            std::move(bodyExpr),
            true,
            fnLoc
        );
    } else if (currentToken.type == TOK_LBRACE) {
        functionDepth++;
        auto body = parseBlock();
        functionDepth--;
        return std::make_unique<FunctionLiteralAST>(
            std::move(params),
            std::move(captures),
            std::move(body),
            false,
            fnLoc
        );
    } else {
        errorHere("Expected '=>' or '{' after function parameters");
    }
}

std::vector<CapturedVariable> Parser::parseCaptureClause() {
    // currentToken is 'mut'; next must be '(' to form a capture clause
    getNextToken(); // consume 'mut'

    if (currentToken.type != TOK_LPAREN)
        errorHere("Expected '(' after 'mut' in capture clause");
    getNextToken(); // consume '('

    std::vector<CapturedVariable> captures;
    if (currentToken.type != TOK_RPAREN) {
        if (currentToken.type != TOK_IDENTIFIER)
            errorHere("Expected variable name in capture clause");
        captures.push_back({currentToken.value, true, currentToken.range.start});
        getNextToken(); // consume identifier

        while (currentToken.type == TOK_COMMA) {
            getNextToken(); // consume ','
            if (currentToken.type == TOK_RPAREN)
                errorHere("Trailing comma in capture clause");
            if (currentToken.type != TOK_IDENTIFIER)
                errorHere("Expected variable name in capture clause");
            captures.push_back({currentToken.value, true, currentToken.range.start});
            getNextToken(); // consume identifier
        }
    }

    if (currentToken.type != TOK_RPAREN)
        errorHere("Expected ')' after capture clause");
    getNextToken(); // consume ')'

    return captures;
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    SourceLocation retLoc = currentToken.range.start;
    if (functionDepth == 0)
        errorHere("'return' outside of function body");
    getNextToken(); // consume 'return'

    // bare return (no value)
    if (currentToken.type == TOK_SEMICOLON) {
        getNextToken(); // consume ';'
        return std::make_unique<ReturnStmtAST>(nullptr, retLoc);
    }

    auto value = parseExpression();
    if (!value) return nullptr;

    if (currentToken.type != TOK_SEMICOLON)
        errorAt("Expected ';' after return statement", previousToken.range.end);
    getNextToken(); // consume ';'

    return std::make_unique<ReturnStmtAST>(std::move(value), retLoc);
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
        case TOK_FN:
            return parseFunctionLiteral();
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
        case TOK_RETURN:
            return parseReturnStatement();
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

std::unique_ptr<ProgramAST> Parser::parseProgram() {
    std::vector<std::unique_ptr<ImportStmtAST>> imports;
    std::vector<std::unique_ptr<ExportStmtAST>> exports;
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (currentToken.type != TOK_EOF) {
        try {
            if (currentToken.type == TOK_IMPORT) {
                auto imp = parseImportStatement();
                if (imp) imports.push_back(std::move(imp));
            } else if (currentToken.type == TOK_EXPORT) {
                auto exp = parseExportStatement();
                if (exp) exports.push_back(std::move(exp));
            } else {
                auto stmt = parseStatement();
                if (stmt) {
                    statements.push_back(std::move(stmt));
                } else {
                    throw std::runtime_error("Failed to parse statement");
                }
            }
        } catch (const ParseError&) {
            throw;
        } catch (const std::runtime_error& e) {
            throw ParseError(std::string(e.what()), currentToken.range.start);
        }
    }
    
    return std::make_unique<ProgramAST>(std::move(imports), std::move(exports), std::move(statements));
}
std::unique_ptr<ImportStmtAST> Parser::parseImportStatement() {
    SourceLocation loc = currentToken.range.start;
    getNextToken(); // consume 'import'
    
    ImportType importType;
    std::vector<ImportedSymbol> symbols;
    std::string alias;
    
    if (currentToken.type == '*') {
        importType = ImportType::Namespace;
        getNextToken(); // consume '*'
        if (currentToken.type != TOK_AS)
            errorHere("Expected 'as' after '*'");
        getNextToken(); // consume 'as'
        if (currentToken.type != TOK_IDENTIFIER)
            errorHere("Expected identifier after 'as'");
        alias = currentToken.value;
        getNextToken(); // consume identifier
    } else if (currentToken.type == TOK_LBRACE) {
        importType = ImportType::Named;
        getNextToken(); // consume '{'
        
        while (currentToken.type != TOK_RBRACE && currentToken.type != TOK_EOF) {
            if (currentToken.type != TOK_IDENTIFIER && currentToken.type != TOK_MUT)
                errorHere("Expected identifier or 'mut' in import list");
                
            // If mut, we might want to support it: "import { mut x }" but let's just parse identifier for now.
            // Oh, specs says: import { mut global_counter }
            bool is_mut = false;
            if (currentToken.type == TOK_MUT) {
                is_mut = true;
                getNextToken();
            }
            
            if (currentToken.type != TOK_IDENTIFIER)
                errorHere("Expected identifier in import list");
            
            std::string name = currentToken.value;
            std::string symAlias;
            getNextToken(); // consume identifier
            
            if (currentToken.type == TOK_AS) {
                getNextToken(); // consume 'as'
                if (currentToken.type != TOK_IDENTIFIER)
                    errorHere("Expected identifier after 'as'");
                symAlias = currentToken.value;
                getNextToken(); // consume identifier
            }
            
            // To simplify, ImportedSymbol doesn't have is_mut flag yet, but let's assume mut goes with name if we change ast.h.
            symbols.push_back({name, symAlias});
            
            if (currentToken.type == TOK_COMMA) {
                getNextToken(); // consume ','
            } else if (currentToken.type != TOK_RBRACE) {
                errorHere("Expected ',' or '}' in import list");
            }
        }
        
        if (currentToken.type != TOK_RBRACE)
            errorHere("Expected '}' after import list");
        getNextToken(); // consume '}'
    } else if (currentToken.type == TOK_IDENTIFIER) {
        // Could be default import or default + named: import default_logger, { PI } from "math"
        // Let's just implement simple default import for now
        importType = ImportType::Default;
        alias = currentToken.value;
        getNextToken(); // consume identifier
        
        if (currentToken.type == TOK_COMMA) {
            // Mixed import: import default_logger, { PI }
            getNextToken(); // consume ','
            if (currentToken.type != TOK_LBRACE)
                errorHere("Expected '{' after ',' in mixed import");
            // For now, simplify and just say mixed imports might need more AST support
            errorHere("Mixed imports are not fully supported yet");
        }
    } else {
        errorHere("Expected '*', '{', or identifier after 'import'");
    }
    
    if (currentToken.type != TOK_FROM)
        errorHere("Expected 'from' in import statement");
    getNextToken(); // consume 'from'
    
    if (currentToken.type != TOK_STRING)
        errorHere("Expected string literal module path");
    std::string moduleName = currentToken.value;
    getNextToken(); // consume string
    
    if (currentToken.type != TOK_SEMICOLON)
        errorAt("Expected ';' after import statement", previousToken.range.end);
    getNextToken(); // consume ';'
    
    return std::make_unique<ImportStmtAST>(moduleName, importType, std::move(symbols), alias, loc);
}

std::unique_ptr<ExportStmtAST> Parser::parseExportStatement() {
    SourceLocation loc = currentToken.range.start;
    getNextToken(); // consume 'export'
    
    if (currentToken.type == TOK_LBRACE) {
        // export { a, b as c }
        getNextToken(); // consume '{'
        std::vector<ExportedSymbol> symbols;
        while (currentToken.type != TOK_RBRACE && currentToken.type != TOK_EOF) {
            if (currentToken.type != TOK_IDENTIFIER)
                errorHere("Expected identifier in export list");
            std::string name = currentToken.value;
            std::string alias;
            getNextToken();
            if (currentToken.type == TOK_AS) {
                getNextToken();
                if (currentToken.type != TOK_IDENTIFIER)
                    errorHere("Expected identifier after 'as'");
                alias = currentToken.value;
                getNextToken();
            }
            symbols.push_back({name, alias});
            if (currentToken.type == TOK_COMMA) {
                getNextToken();
            } else if (currentToken.type != TOK_RBRACE) {
                errorHere("Expected ',' or '}' in export list");
            }
        }
        if (currentToken.type != TOK_RBRACE)
            errorHere("Expected '}'");
        getNextToken();
        
        if (currentToken.type != TOK_SEMICOLON)
            errorAt("Expected ';' after export list", previousToken.range.end);
        getNextToken();
        
        return std::make_unique<ExportStmtAST>(ExportType::Named, std::move(symbols), nullptr, loc);
    } else if (currentToken.type == TOK_DEFAULT) {
        getNextToken(); // consume 'default'
        auto expr = parseExpression();
        if (!expr) return nullptr;
        if (currentToken.type != TOK_SEMICOLON)
            errorAt("Expected ';' after default export", previousToken.range.end);
        getNextToken();
        return std::make_unique<ExportStmtAST>(ExportType::Default, std::vector<ExportedSymbol>{}, std::move(expr), loc);
    } else if (currentToken.type == TOK_FN) {
        // export fn foo() {}
        // Wait, parseStatement doesn't handle named functions currently?
        // Ah, `fn` usually returns a FunctionLiteralAST which is an expression.
        // Wait, what if it's `export fn is_even(n) { ... }` ?
        // We can parse it by parsing an assignment or we can parse it as an expression.
        // If we look at specs/modules.md, it says `export fn is_even(n) { ... }`.
        errorHere("Named function exports are not fully implemented yet");
    } else {
        // Try parsing assignment: export var = 1;
        // The rest of the line is a statement
        auto stmt = parseStatement();
        if (!stmt) return nullptr;
        return std::make_unique<ExportStmtAST>(ExportType::Assignment, std::vector<ExportedSymbol>{}, std::move(stmt), loc);
    }
}
