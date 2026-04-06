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
