# System Patterns: ArithLang Architecture

## Overall Architecture

### Four-Phase Compiler Pipeline
```
Source Code → Lexer → Parser → AST → CodeGen → LLVM IR → Execution
```

Each phase is cleanly separated with defined interfaces, enabling independent testing and development.

## Core Components

### 1. Lexer (`src/lexer.cpp`, `include/lexer.h`)
**Purpose**: Tokenize source code into meaningful symbols
**Pattern**: Iterator-based token stream
**Key Classes**: 
- `Token`: Represents individual language elements
- `Lexer`: Stateful tokenizer with lookahead

### 2. Parser (`src/parser.cpp`, `include/parser.h`)
**Purpose**: Build Abstract Syntax Tree from token stream
**Pattern**: Recursive Descent Parser with operator precedence
**Key Classes**:
- `Parser`: Manages parsing state and AST construction
- Implements precedence climbing for operators

### 3. AST Nodes (`include/ast.h`)
**Purpose**: Represent program structure in memory
**Pattern**: Visitor pattern for code generation
**Key Classes**:
- `ASTNode`: Base class with virtual `codegen()` method
- `ExprAST`: Base for all expressions
- `NumberExprAST`, `VariableExprAST`, `BinaryExprAST`: Specific expression types
- `AssignmentExprAST`, `PrintExprAST`: Statement types
- `IfExprAST`, `WhileExprAST`: Control flow

### 4. Code Generator (`src/codegen.cpp`, `include/codegen.h`)
**Purpose**: Convert AST to LLVM IR
**Pattern**: Visitor pattern with LLVM builder
**Key Components**:
- LLVM context and module management
- Symbol table for variable tracking
- Type system (primarily double floating-point)

## Design Patterns

### 1. Visitor Pattern (Code Generation)
Each AST node implements `codegen()` method that:
- Generates appropriate LLVM IR
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
