````markdown
# Parse Error Reporting Specification

## Overview
When a parsing error occurs in ArithLang, the compiler should report the exact source location with:
- File path
- Line number (1-based)
- Column number (1-based, character position)
- Clear, actionable error message

Optionally, the CLI should render a source snippet line with a caret (^) pointing at the column.

This document defines requirements, design, implementation guidelines, code samples, and example error outputs for consistent parse-time error reporting.

## Goals and Non-goals

Goals:
- Consistent error format across all parser errors
- Include file, line, column in every parser error
- Provide snippet + caret rendering in CLI path
- Minimal changes to public APIs, keep compatibility

Non-goals (out of scope for this change):
- Full multi-error recovery (stop-at-first-error remains)
- Elaborate suggestions/fixes beyond current messages
- Adding locations to type-checking errors (can be a follow-up)

## Requirements

R1. Every parsing error must include file, line, column.

R2. Standard error text format (for tooling friendliness):
```
<file>:<line>:<column>: error: <message>
```

R3. CLI should print an additional source snippet line and a caret line indicating the column by default:
```
<file>:<line>:<column>: error: <message>
<source line>
<spaces>^
```

R4. Backward-compatibility for existing tests that match substrings of the message. Core messages like "Expected ';' after print statement" must remain unchanged; we only add a standardized prefix and optional snippet lines.

R5. Line/column accounting must handle newlines correctly, including inside string literals. Tabs count as a single column (simple alignment); we may improve later.

R6. Errors at EOF should report the last known location (line end) and indicate EOF context where helpful.

## Design

### Data model additions

```cpp
struct SourceLocation {
    std::string file;  // absolute or provided path
    int line = 1;      // 1-based
    int column = 1;    // 1-based (UTF-8 byte index; future: grapheme-aware)
};

struct SourceRange {
    SourceLocation start;
    SourceLocation end; // one past last character (exclusive end position)
    
    // For single-character tokens: end.column = start.column + 1
    // For multi-character tokens: end.column = start.column + token_length
    // For multi-line tokens: end.line > start.line
};
```

Extend `Token` to carry its source span:

```cpp
struct Token {
    TokenType type;
    std::string value;
    double numValue;
    SourceRange range;           // NEW

    Token(TokenType t, const std::string& v = "", double n = 0.0,
          SourceRange r = {})
        : type(t), value(v), numValue(n), range(std::move(r)) {}
};
```

### Lexer updates

- Track `filename`, `line`, and `column` as the lexer advances.
- On each tokenization, capture `start = currentLocation()` before consuming characters, and `end = currentLocation()` after.
- Populate `Token::range` with `{start, end}`.
- **Line/Column Update Rules**:
  - Increment `line` and reset `column` to 1 on `\n`
  - Increment `column` on other characters (including `\t`, which counts as 1 column)
  - Multi-byte UTF-8 characters increment column by 1 (byte-based counting for simplicity)
- **EOF Token**: Create EOF token with range.start = range.end = currentLocation()
- Support construction with a `filename` (optional, defaults to `"<stdin>"`).

Pseudo-API additions:

```cpp
class Lexer {
public:
    Lexer(const std::string& input, std::string filename = "<stdin>");
    const std::string& getFilename() const;
    SourceLocation currentLocation() const; // location of currentChar
    // ...existing members...
};
```

### Parser error type

Introduce a dedicated exception type carrying the location:

```cpp
class ParseError : public std::runtime_error {
public:
    SourceLocation loc;
    explicit ParseError(const std::string& message, SourceLocation where)
        : std::runtime_error(message), loc(std::move(where)) {}
};
```

Helper inside `Parser` to throw with the current token's start location by default:

```cpp
[[noreturn]] inline void errorHere(const std::string& msg) {
    // Use current token's start position for most errors
    SourceLocation loc = currentToken.range.start;
    
    // Special case: if current token is EOF, use its location directly
    // (EOF token's range.start should be at the actual end of file)
    throw ParseError(msg, loc);
}

[[noreturn]] inline void errorAt(const std::string& msg, SourceLocation loc) {
    // For errors at specific locations (e.g., after consuming tokens)
    throw ParseError(msg, loc);
}
```

Replace `throw std::runtime_error("...")` call sites in `parser.cpp` with `throw ParseError("...", location)` using appropriate location context:

**Error Location Guidelines**:
- **Unexpected token errors**: Use `currentToken.range.start` to point at the unexpected token
- **Missing token errors** (e.g., missing `)` after `(`): Use the location after the last successfully parsed token
- **EOF errors**: Use `currentToken.range.start` (EOF token's position)
- **Semantic errors** (e.g., invalid assignment target): Use the problematic token's start position

**Examples**:
```cpp
// Unexpected token while expecting semicolon
if (currentToken.type != TOK_SEMICOLON)
    errorHere("Expected ';' after print statement");

// Missing closing parenthesis - point after the opening paren
auto openParenLoc = currentToken.range.start;
getNextToken(); // consume '('
auto expr = parseExpression();
if (currentToken.type != TOK_RPAREN)
    errorAt("Expected ')'", currentToken.range.start); // points at unexpected token

// EOF in unterminated string - lexer should handle this
// when creating the string token, set range to point at opening quote
```

### CLI reporting (main)

Catch `ParseError` early in `main.cpp` and render a consistent error output. For other exceptions (`std::runtime_error`), keep the current behavior.

```cpp
int main(int argc, char** argv) {
    // ...load source into std::string code and determine filename...
    try {
        Lexer lex(code, filename);
        Parser parser(lex);
        auto program = parser.parseProgram();
        // ... typeCheck, codegen, etc. ...
    } catch (const ParseError& e) {
        printParseError(e, code); // see below
        return 1;
    } catch (const std::runtime_error& e) {
        // Back-compat path for other runtime errors
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
```

#### Error printing helper

```cpp
// Helper function to extract a specific line from source
static std::string getLine(const std::string& source, int lineNumber) {
    if (lineNumber < 1) return "";
    
    int currentLine = 1;
    size_t lineStart = 0;
    
    // Find the start of the target line
    for (size_t i = 0; i < source.length() && currentLine < lineNumber; ++i) {
        if (source[i] == '\n') {
            currentLine++;
            lineStart = i + 1;
        }
    }
    
    if (currentLine != lineNumber) return ""; // Line not found
    
    // Find the end of the line
    size_t lineEnd = source.find('\n', lineStart);
    if (lineEnd == std::string::npos) lineEnd = source.length();
    
    return source.substr(lineStart, lineEnd - lineStart);
}

static void printParseError(const ParseError& e, const std::string& source) {
    const auto& L = e.loc;
    std::fprintf(stderr, "%s:%d:%d: error: %s\n",
                 L.file.c_str(), L.line, L.column, e.what());

    // Render the source line and caret (best-effort; guard indices)
    const auto lineStr = getLine(source, L.line); // 1-based
    if (!lineStr.empty()) {
        std::fprintf(stderr, "%s\n", lineStr.c_str());
        
        // Create caret line with proper alignment
        std::string caret;
        for (int i = 1; i < L.column; ++i) {
            // Use space for all characters (tabs treated as single column)
            caret.push_back(' ');
        }
        caret.push_back('^');
        std::fprintf(stderr, "%s\n", caret.c_str());
    }
}
```

**Column Alignment Policy**:
- Tabs count as 1 column for position tracking
- Caret alignment uses spaces only (no tab expansion in Phase 1)
- Very long lines (>120 chars) are displayed in full (truncation is future work)

## Code Samples (minimal diffs)

The following excerpts illustrate the core changes. These are not full files—apply them in context.

### `include/lexer.h` (additions)

```cpp
struct SourceLocation {
    std::string file;
    int line = 1;
    int column = 1;
};

struct SourceRange { SourceLocation start, end; };

struct Token {
    TokenType type;
    std::string value;
    double numValue;
    SourceRange range; // NEW
    Token(TokenType t, const std::string& v = "", double n = 0.0, SourceRange r = {})
        : type(t), value(v), numValue(n), range(std::move(r)) {}
};

class Lexer {
    // ...existing...
private:
    std::string filename;
    int line = 1;
    int column = 1;
    
public:
    Lexer(const std::string& input, std::string filename = "<stdin>");
    SourceLocation currentLocation() const;
    const std::string& getFilename() const;
    
    // Helper for creating EOF tokens
    Token makeEOFToken() {
        auto loc = currentLocation();
        return Token(TOK_EOF, "", 0.0, {loc, loc});
    }
};
```

### `include/parser.h` (additions)

```cpp
class ParseError : public std::runtime_error {
public:
    SourceLocation loc;
    explicit ParseError(const std::string& message, SourceLocation where)
        : std::runtime_error(message), loc(std::move(where)) {}
};

class Parser {
    // ...existing...
    [[noreturn]] void errorHere(const std::string& msg);
    [[noreturn]] void errorAt(const std::string& msg, SourceLocation loc);
};
```

### `src/parser.cpp` (usage example)

```cpp
std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    auto openParenLoc = currentToken.range.start;
    getNextToken(); // consume '('
    auto v = parseExpression();
    if (!v) return nullptr;
    if (currentToken.type != TOK_RPAREN)
        errorAt("Expected ')'", currentToken.range.start); // point at unexpected token
    getNextToken();
    return v;
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    getNextToken(); // consume 'print'
    auto firstExpr = parseExpression();
    if (!firstExpr) return nullptr;
    // ... parse optional args ...
    if (currentToken.type != TOK_SEMICOLON)
        errorHere("Expected ';' after print statement"); // point at current token
    getNextToken();
    // ...
}
```

### `src/main.cpp` (catch and print)

```cpp
try {
    Lexer lexer(source, filename);
    Parser parser(lexer);
    auto program = parser.parseProgram();
} catch (const ParseError& e) {
    printParseError(e, source);
    return 1;
}
```

## Example Error Messages

Assume file `examples/my.k` contains:

1) Missing semicolon after print
```
print 42
```
Output:
```
examples/my.k:1:8: error: Expected ';' after print statement
print 42
       ^
```

2) Unmatched parenthesis
```
x = (10 + 5;
```
Output:
```
examples/my.k:1:4: error: Expected ')' 
x = (10 + 5;
   ^
```

3) Invalid assignment target
```
123 = 42;
```
Output:
```
examples/my.k:1:1: error: Invalid assignment target
123 = 42;
^
```

4) Unterminated string literal at EOF
```
print "Hello
```
Output:
```
examples/my.k:1:7: error: Unterminated string literal
print "Hello
      ^
```

**Note**: Lexer should detect unterminated strings and create a token with range pointing to the opening quote.

5) Expected ';' after expression statement (inside block)
```
if (x > 0) { print x }
```
Output:
```
examples/my.k:1:20: error: Expected ';' after expression statement
if (x > 0) { print x }
                   ^
```

## Edge Cases and Notes

### Column Calculation Details
- **Tabs**: Count as 1 column for position tracking (simple but consistent)
- **Multi-byte UTF-8**: Each UTF-8 character counts as 1 column (byte-based counting)
- **Carriage Return**: `\r\n` and `\r` both reset column to 1 and increment line

### EOF Handling Strategy
- **EOF Token Creation**: Lexer creates EOF token with `range.start = range.end = currentLocation()`
- **Parser EOF Errors**: Use `currentToken.range.start` when currentToken is EOF
- **Lexer EOF Errors**: For unterminated constructs, point to the opening character

### Multi-character Operators
- **Token Range**: For `>=`, `<=`, `==`, `!=`, etc.
  - `range.start` = position of first character
  - `range.end` = position after last character
- **Example**: For `>=` at column 5: start.column=5, end.column=7

### String Literal Handling
- **Newlines in strings**: Line/column tracking continues through `\n` inside strings
- **Escape sequences**: `\"`, `\\`, etc. count as 2 columns each
- **Unterminated strings**: Lexer should detect and report error with range pointing to opening quote

### Error Recovery (Phase 1)
- **Stop-at-first-error**: Parser stops on first ParseError (no recovery)
- **Safe termination**: Ensure no dangling resources or undefined behavior
- **Future enhancement**: Synchronization points for multi-error reporting

## Backward Compatibility

### Test Compatibility Strategy
- **Core message preservation**: Error messages like "Expected ';' after print statement" remain unchanged
- **Prefix addition**: New `file:line:column: error:` prefix won't break substring matching
- **Snippet lines**: Additional source snippet and caret lines are additive
- **Test updates needed**: Tests that check exact error output format will need updates

### Migration Path
1. **Phase 1**: Add location tracking, preserve existing error messages
2. **Update tests**: Modify tests to expect new format or use substring matching
3. **Phase 2**: Enhance error messages with additional context if needed

### Compiler Flags (Future)
Consider adding flags for different error formats:
- `--error-format=short`: Messages only (current behavior)
- `--error-format=rich`: Full prefix + snippet + caret (new default)
- `--error-format=json`: Machine-readable format for IDE integration

## Implementation Steps

### Phase 1: Core Location Tracking (MVP)
1. **Data Structures**: Add `SourceLocation`/`SourceRange` structs
2. **Token Extension**: Extend `Token` to carry `range` field
3. **Lexer Updates**: 
   - Track `filename`, `line`, `column` during tokenization
   - Populate `Token::range` for all tokens including EOF
   - Handle newlines and special characters correctly
4. **ParseError Type**: Add `ParseError` exception class with location
5. **Parser Updates**: Replace `std::runtime_error` throws with `ParseError`
6. **Basic CLI Reporting**: Add simple `file:line:column: error: message` format

### Phase 2: Enhanced Error Display
7. **Source Snippet**: Implement `getLine()` helper and caret rendering
8. **Error Context**: Improve error messages with better location context
9. **Test Updates**: Update test expectations for new error format

### Phase 3: Advanced Features (Future)
10. **Tab Expansion**: Visual alignment with configurable tab width
11. **Colored Output**: Syntax highlighting for error context
12. **Multiple Errors**: Error recovery and batch reporting
13. **IDE Integration**: JSON error format for language servers

### Testing Strategy
- **Unit Tests**: Test `SourceLocation` tracking in isolation
- **Lexer Tests**: Verify correct range calculation for all token types
- **Parser Tests**: Verify error locations point to correct positions
- **Integration Tests**: End-to-end error reporting with real source files
- **Regression Tests**: Ensure existing error detection still works

## Future Work

### Enhanced Error Reporting
- **Location-aware type-checking errors**: Attach `SourceLocation` to AST nodes
- **Multiple-error recovery**: Implement synchronization points and continue parsing
- **Better error messages**: Context-aware suggestions and fix hints
- **Error categories**: Distinguish between syntax, semantic, and lexical errors

### Visual Improvements  
- **Colored output**: Syntax highlighting and colored error indicators
- **Tab expansion**: Configurable tab width for proper visual alignment
- **Line truncation**: Smart truncation for very long lines
- **Multi-line errors**: Better handling of errors spanning multiple lines

### Developer Experience
- **IDE Integration**: Language Server Protocol support with structured error data
- **JSON Error Format**: Machine-readable error output for tooling
- **Error recovery**: Continue parsing after errors to find more issues
- **Grapheme-aware column calculation**: Proper Unicode handling for international text

### Performance Optimizations
- **Line caching**: Cache line splits for better performance with large files
- **Incremental parsing**: Update only changed portions for IDE integration
- **Memory efficiency**: Optimize SourceLocation storage for large codebases

````
