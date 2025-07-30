# System Patterns: ArithLang Architecture

## Architecture Overview

ArithLang follows a **traditional compiler pipeline** with modern C++ practices:

```
Source Code (.k) → Lexer → Parser → AST → CodeGen → LLVM IR → JIT Execution
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

### 2. Parser (Syntax Analysis)
**Location**: `src/parser.cpp`, `include/parser.h`
**Pattern**: Recursive descent with operator precedence

**Key Design Decisions**:
- **Recursive descent**: Natural mapping to grammar rules
- **Precedence climbing**: Efficient operator precedence handling
- **Program-level parsing**: `parseProgram()` as primary public interface
- **Private implementation**: `parseStatement()` encapsulated for internal use
- **Grammar compliance**: Perfect alignment with BNF specification in syntax.md

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

### 3. AST (Abstract Syntax Tree)
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

### 4. Code Generation
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

## Error Handling Patterns

### Lexer Error Pattern
```cpp
// Position-aware error reporting
if (current >= input.length()) {
    return Token{TokenType::EOF_TOKEN, "", line, column};
}
```

### Parser Error Pattern
```cpp
// Structured error with context
throw std::runtime_error("Expected ';' after statement at line " + 
                         std::to_string(currentToken.line));
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

### Git Workflow Pattern
```bash
# Atomic commits for specific improvements
git commit -m "refactor: reduce main function from 70 to 22 lines"
git commit -m "refactor: optimize lexer getNextToken function"
git commit -m "cleanup: remove unused Parser::parse function"
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
- Returns `llvm::Value*` representing the computed value
- Handles type conversions and error cases

### 2. Recursive Descent Parsing
Parser methods correspond to grammar rules:
- `parseExpression()` → handles operator precedence
- `parseStatement()` → processes different statement types
- `parseBlock()` → manages scoped statement groups

### 3. Builder Pattern (LLVM Integration)
Uses LLVM's IRBuilder for instruction generation:
- Context management for LLVM types and constants
- Module as container for generated functions
- Builder for instruction sequencing

## Critical Implementation Decisions

### Memory Management
- **Smart Pointers**: Extensive use of `std::unique_ptr` for AST nodes
- **LLVM Ownership**: LLVM manages its own object lifecycles
- **No Manual Memory**: Avoid raw pointers except for LLVM interfaces

### Error Handling
- **Parse Errors**: Return nullptr from parsing methods
- **Runtime Errors**: Use LLVM's error reporting mechanisms
- **Type Errors**: Handled during code generation phase

### Symbol Management
- **Global Scope**: Currently single scope for variables
- **Type Consistency**: All variables are double precision floats
- **Forward References**: Not supported (variables must be defined before use)

## Component Relationships

### Dependency Flow
```
main.cpp → parser.h → lexer.h → ast.h → codegen.h
    ↓         ↓         ↓         ↓         ↓
   CLI    Parsing   Tokenizing  AST     LLVM IR
```

### Data Flow
1. **Source** → Lexer → **Tokens**
2. **Tokens** → Parser → **AST**
3. **AST** → CodeGen → **LLVM IR**
4. **LLVM IR** → Execution → **Results**

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
