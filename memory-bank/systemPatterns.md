# System Patterns: ArithLang Architecture

## Architecture Overview

ArithLang follows a **traditional compiler pipeline** with modern C++ and comprehensive type safety:

```
Source Code (.k) → Lexer → Parser → Type Checker → AST → CodeGen → LLVM IR → JIT Execution
```

Each phase has **clear separation of concerns** and is independently testable.

## Core Components

### 1. Lexer (Tokenization)
**Location**: `src/lexer.cpp`, `include/lexer.h`
**Pattern**: State machine with character-by-character processing

**Key Design Decisions**:
- **Single-character lookahead**: Simple and efficient
- **Position tracking**: Line/column information for error reporting
- **Token enumeration**: Strongly typed token identification
- **Refactored Architecture**: Helper functions for maintainability

**Recent Improvements**:
```cpp
// Before: 60-line monolithic getNextToken()
Token getNextToken() { /* 60 lines of mixed logic */ }

// After: 26-line main function with helpers
Token getNextToken() {
    // Core logic
    return handleKeywordOrIdentifier() || handleOperator() || ...
}

Token handleKeywordOrIdentifier() { /* focused logic */ }
Token handleOperator() { /* focused logic */ }
```

### 2. Type Checker (Type Safety)
**Location**: `src/type_check.cpp`, `include/type_check.h`
**Pattern**: Recursive AST validation with type safety enforcement

**Key Design Decisions**:
- **Pipeline Integration**: Positioned between parsing and code generation
- **Recursive Validation**: Traverses all AST nodes for comprehensive type checking
- **String Type Safety**: Prevents string literals in arithmetic/comparison contexts
- **Clean Interface**: Simple `void typeCheck(ASTNode* node)` function
- **Error Reporting**: Clear, specific error messages for type violations

**Implementation Pattern**:
```cpp
void typeCheck(ASTNode* node) {
    if (!node) return;
    
    // Binary Operation Type Safety
    if (auto bin = dynamic_cast<BinaryExprAST*>(node)) {
        typeCheck(bin->getLHS());
        typeCheck(bin->getRHS());
        if (dynamic_cast<StringLiteralAST*>(bin->getLHS())) {
            throw std::runtime_error("String literal cannot be used in binary operation");
        }
        return;
    }
    
    // Recursive validation for all node types...
}
```

**Integration in Main Pipeline**:
```cpp
// src/main.cpp compilation pipeline
auto programAST = parser.parseProgram();
typeCheck(programAST.get());  // Type safety validation
auto result = programAST->codegen();  // Proceed to code generation
```

### 3. Parser (Syntax Analysis)
**Location**: `src/parser.cpp`, `include/parser.h`
**Pattern**: Recursive descent with operator precedence

**Key Design Decisions**:
- **Recursive descent**: Natural mapping to grammar rules
- **Precedence climbing**: Efficient operator precedence handling
- **Program-level parsing**: `parseProgram()` as primary public interface
- **Private implementation**: `parseStatement()` encapsulated for internal use
- **Grammar compliance**: Perfect alignment with BNF specification in syntax.md
- **Strict syntax validation**: Required semicolons with comprehensive error handling

**Recent Critical Improvements**:
```cpp
// Before: Optional semicolon handling (BUG)
if (currentToken.type == TOK_SEMICOLON) {
    getNextToken();  // Optional semicolon consumption
}

// After: Required semicolon validation (FIXED)
if (currentToken.type != TOK_SEMICOLON) {
    // caret just after the last expr token
    errorAt("Expected ';' after expression statement", previousToken.range.end);
}
getNextToken(); // Required semicolon consumption
```

**Error Handling Enhancement**:
```cpp
// Enhanced parseProgram() with precise locations
try {
    auto stmt = parseStatement();
    if (stmt) {
        statements.push_back(std::move(stmt));
    } else {
        throw std::runtime_error("Failed to parse statement");
    }
} catch (const ParseError&) {
    throw; // preserve location-rich errors
} catch (const std::runtime_error& e) {
    throw ParseError(std::string(e.what()), currentToken.range.start);
}
```

**Grammar Structure**:
```
program → statement*
statement → assignment | print | if | while | expression
expression → comparison | arithmetic | primary
```

**New Architecture Pattern**:
```cpp
// Program-level compilation
std::unique_ptr<ASTNode> parseProgram() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (currentToken.type != TOK_EOF) {
        auto stmt = parseStatement();  // private method
        if (stmt) statements.push_back(std::move(stmt));
    }
    return std::make_unique<ProgramAST>(std::move(statements));
}
```

### 4. AST (Abstract Syntax Tree)
**Location**: `include/ast.h`
**Pattern**: Composite pattern with visitor support

**Memory Management Pattern**:
```cpp
// Smart pointer ownership
std::unique_ptr<ASTNode> left;
std::unique_ptr<ASTNode> right;

// Factory pattern for creation
static std::unique_ptr<NumberNode> create(double value);
```

**Node Hierarchy**:
- **Program Node**: `ProgramAST` (top-level container for statements)
- **Expression Nodes**: `NumberExprAST`, `VariableExprAST`, `BinaryExprAST`
- **Statement Nodes**: `PrintStmtAST`, `IfStmtAST`, `WhileStmtAST`, `BlockAST`
- **Assignment**: `AssignmentExprAST` (bridge between expression and statement)

### 5. Code Generation
**Location**: `src/codegen.cpp`, `include/codegen.h`
**Pattern**: Visitor pattern over AST nodes

**LLVM Integration Pattern**:
```cpp
// Context and module management
llvm::LLVMContext context;
std::unique_ptr<llvm::Module> module;
llvm::IRBuilder<> builder;

// Symbol table for variables
std::map<std::string, llvm::Value*> namedValues;
```

**High-Precision Output Pattern**:
```cpp
// PrintStmtAST::codegen() - Enhanced precision implementation
llvm::Value* PrintStmtAST::codegen() {
    llvm::Value* val = expr->codegen();
    if (!val) return nullptr;
    
    // High-precision format string for scientific accuracy
    llvm::Constant* formatStr = codeGenInstance->getBuilder()
        .CreateGlobalString("%.15f\n");  // 15-digit precision
    
    // printf call with enhanced precision
    std::vector<llvm::Value*> args = {formatStr, val};
    return codeGenInstance->getBuilder().CreateCall(printfFunc, args, "printfcall");
}
```

**Formatted Print Pattern**:
- 지원 포맷: `%f`, `%g`, `%e`, `%d`, `%s`, `%%`, 그리고 `%.Nf` 정밀도
- `%d`: double → i32 변환 후 출력
- `%s`: 문자열 리터럴만 허용(비리터럴은 예외)
- `%%`: 리터럴 `%`
- 문자열 단독 출력: `processedString`을 만들고 `%s`로 출력(자동 개행 없음)

## Refactoring Patterns Applied

### Function Length Management
**Target**: Keep functions under 40 lines for maintainability

**Main Function Refactoring**:
```cpp
// Before: 70-line main() with mixed responsibilities
int main(int argc, char* argv[]) {
    // Command line parsing
    // LLVM setup  
    // Compilation
    // File I/O
    // All mixed together - 70 lines
}

// After: 22-line main() with clear separation
int main(int argc, char* argv[]) {
    auto args = parseCommandLine(argc, argv);
    auto module = setupLLVMFunction();
    compileSource(args.filename, module.get());
    saveIRToFile(module.get(), args.output);
    return 0;
}
```

### Single Responsibility Principle
Each extracted function has **one clear purpose**:
- `parseCommandLine()`: CLI argument processing
- `setupLLVMFunction()`: LLVM initialization
- `compileSource()`: Compilation pipeline
- `saveIRToFile()`: Output generation

### Dead Code Elimination Pattern
**Applied to Parser class**:
```cpp
// Removed: Unused Parser::parse() function
// Kept: Active parseStatement() interface
// Result: Cleaner API with single responsibility
```

## Build System Architecture

### CMake Pattern
**Modern CMake practices** with:
- Target-based dependency management
- External dependency integration (LLVM, Google Test)
- Multiple executable targets
- Cross-platform compatibility

```cmake
# LLVM integration
find_package(LLVM REQUIRED CONFIG)
target_link_libraries(arithc ${llvm_libs})

# Test framework
FetchContent_Declare(googletest ...)
FetchContent_MakeAvailable(googletest)
```

### Testing Architecture
**Comprehensive test strategy**:
- **Unit Tests**: Component isolation testing
- **Integration Tests**: End-to-end pipeline testing
- **System Tests**: Complete program execution
- **Automated**: All tests run on build

## Diagnostics

### Professional-Grade Error Location Tracking ✅ **UPGRADED**
ArithLang은 전문가급 정확도의 에러 위치 추적을 제공합니다.

**Complete AST Location Information**:
- **All AST nodes** now carry precise location information
- `NumberExprAST`, `StringLiteralAST`: Literal location tracking
- `UnaryExprAST`, `BinaryExprAST`: Operator location tracking  
- `PrintStmtAST`, `IfStmtAST`, `WhileStmtAST`: Keyword location tracking
- **No more fallback logic**: All errors use actual AST node locations

**Parser Location Capture**:
```cpp
// Precise location capture before token consumption
SourceLocation printLoc = currentToken.range.start;
getNextToken(); // consume 'print'
// Location passed to AST constructor
return std::make_unique<PrintStmtAST>(std::move(firstExpr), printLoc);
```

**Type Checker Integration**:
```cpp
// Direct use of AST node locations (no heuristics)
if (t == ValueType::String) {
    throw ParseError("String literal cannot be used in unary operation", 
                     unary->getOperatorLocation()); // Precise location
}
```

**Error Location Test Coverage**:
- 6 comprehensive test cases with EXPECTED comments
- All scenarios: binary ops, unary ops, control flow, print statements
- Complex nested expressions and operator precedence handling
- 100% test pass rate with accurate location reporting

**CLI 출력 형식**:
- `<file>:<line>:<column>: error: <message>`
- 소스 스니펫 1줄 + 정확한 캐럿(^) 위치

**성과**: 이전의 1:1 fallback 휴리스틱을 완전히 제거하고, 모든 에러가 정확한 위치 정보를 제공

### Lexer Error Pattern
- CRLF/LF 줄바꿈 정규화, `//` 라인 주석 스킵
- 식별자에 0x80 이상(UTF-8 바이트) 허용
- 문자열: 미종결/잘못된 이스케이프에서 `ParseError(currentLocation())`
```cpp
if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
    throw ParseError("Unterminated string literal", currentLocation());
}
```

### Parser Error Pattern
- 세미콜론 누락 시 마지막 토큰 끝에 캐럿을 두기 위해 `previousToken.range.end` 사용
- `parseProgram()`은 `ParseError`는 그대로 전달하고, 그 외 예외는 현재 토큰 위치로 래핑
```cpp
if (currentToken.type != TOK_SEMICOLON) {
    errorAt("Expected ';' after expression statement", previousToken.range.end);
}

try {
    auto stmt = parseStatement();
    if (stmt) statements.push_back(std::move(stmt));
    else throw std::runtime_error("Failed to parse statement");
} catch (const ParseError&) {
    throw; // 위치 보존
} catch (const std::runtime_error& e) {
    throw ParseError(std::string(e.what()), currentToken.range.start);
}
```

## Memory Management Patterns

### RAII with Smart Pointers
```cpp
// Automatic memory management
std::unique_ptr<ASTNode> parseExpression();
std::unique_ptr<IfNode> parseIf();

// Move semantics for efficiency
return std::make_unique<NumberNode>(value);
```

### LLVM Memory Model
```cpp
// LLVM handles IR memory automatically
// No manual cleanup needed for IR objects
llvm::Value* result = builder.CreateAdd(left, right, "addtmp");
```

## Development Environment Patterns

### VSCode Integration
**IntelliSense Configuration**:
```json
// .vscode/c_cpp_properties.json
{
    "includePath": [
        "/opt/homebrew/include/**",
        "/opt/homebrew/Cellar/llvm/20.1.8/include"
    ],
    "compileCommands": "${workspaceFolder}/build/compile_commands.json"
}
```

## Design Principles Applied

1. **Single Responsibility**: Each component has one clear purpose
2. **Open/Closed**: Extensible for new tokens/AST nodes
3. **DRY**: Helper functions eliminate code duplication
4. **KISS**: Simple, readable implementation over complex optimization
5. **YAGNI**: Only implement what's actually needed

## Performance Considerations

### Compilation Speed
- **Fast lexing**: Simple character-by-character processing
- **Efficient parsing**: Minimal backtracking
- **Smart pointers**: Move semantics reduce copying

### Runtime Performance
- **LLVM optimization**: Professional-grade backend
- **JIT compilation**: Direct execution without intermediate files
- **Type specialization**: Double precision throughout

## Critical Implementation Decisions

### Memory Management
- **Smart Pointers**: Extensive use of `std::unique_ptr` for AST nodes
- **LLVM Ownership**: LLVM manages its own object lifecycles
- **No Manual Memory**: Avoid raw pointers except for LLVM interfaces

### Error Handling
- **Exceptions-first**: Lexer/Parser/Type 오류는 예외로 보고
- **Parse/Type Errors**: `ParseError`로 표준화된 위치 정보 포함
- **CLI Diagnostics**: `<file>:<line>:<column>: error: ...` + 스니펫/캐럿 출력

### Symbol Management
- **Global Scope**: Currently single scope for variables
- **Type Consistency**: All variables are double precision floats
- **Forward References**: Not supported (variables must be defined before use)

## Component Relationships

### Dependency Flow
```
main.cpp → parser.h → lexer.h → type_check.h → ast.h → codegen.h
    ↓         ↓         ↓           ↓           ↓         ↓
   CLI    Parsing   Tokenizing  Type Check   AST     LLVM IR
```

### Data Flow
1. **Source** → Lexer → **Tokens**
2. **Tokens** → Parser → **AST**
3. **AST** → Type Checker → **Validated AST**
4. **Validated AST** → CodeGen → **LLVM IR**
5. **LLVM IR** → Execution → **Results**

## Extension Points

### Adding New Operators
1. Add token type in lexer
2. Update parser precedence table
3. Extend BinaryExprAST codegen
4. Add test cases

### Adding New Statement Types
1. Create new AST node class
2. Add parsing method
3. Implement codegen method
4. Update statement parsing dispatcher

### Adding New Data Types
1. Extend type system in codegen
2. Update AST nodes for type information
3. Modify LLVM type mapping
4. Update all codegen methods
