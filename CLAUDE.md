# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ArithLang is an LLVM-based arithmetic expression compiler written in C++17. It follows a classic four-stage compiler pipeline: Lexer → Parser → AST → CodeGen, transforming mathematical expressions into executable LLVM IR.

## Build Commands

```bash
# Standard build process
mkdir -p build && cd build && cmake .. && make

# Rebuild after changes
cd build && make

# Clean rebuild
rm -rf build && mkdir build && cd build && cmake .. && make
```

## Usage

```bash
./build/arithc "2 + 3 * 4"     # Generates LLVM IR for expression
./build/arithc "(10 - 2) / 4"  # Handles parentheses and precedence
```

## Architecture Overview

The codebase implements a classic compiler pipeline with clear separation of concerns:

### 1. Lexer (`lexer.h/cpp`)
- Character-by-character tokenization with lookahead
- Supports: floating-point numbers, identifiers, operators (`+`, `-`, `*`, `/`), parentheses
- Token structure: `{type, value, numValue}` with enum-based types

### 2. Parser (`parser.h/cpp`)
- Operator precedence parsing (Pratt parser style)
- Precedence levels: `*/` (40), `+-` (10)
- Recursive descent with left-associative binary operators
- Error handling via exceptions

### 3. AST (`ast.h`)
- Three node types: `NumberExprAST`, `VariableExprAST`, `BinaryExprAST`
- Virtual `codegen()` method for visitor pattern
- Memory management via `std::unique_ptr`

### 4. CodeGen (`codegen.h/cpp`)
- LLVM IR generation using IRBuilder
- Singleton pattern with global LLVM context/module management
- Floating-point arithmetic instructions (`fadd`, `fsub`, `fmul`, `fdiv`)

### 5. Main Driver (`main.cpp`)
- Command-line interface orchestrating the pipeline
- Creates LLVM function returning double
- Outputs generated IR to stdout

## Development Guidelines

### Adding New Operators
1. Add token type to `TokenType` enum in `lexer.h`
2. Update lexer's `getNextToken()` to recognize the operator
3. Add precedence entry in `Parser` constructor
4. Implement operator logic in `BinaryExprAST::codegen()`

### Extending AST Nodes
- Inherit from `ExprAST` base class
- Implement virtual `codegen()` method
- Add corresponding parser method
- Use smart pointers for memory management

### Code Style
- Modern C++17 with RAII principles
- Smart pointers (`std::unique_ptr`) for AST nodes
- Exception-based error handling
- Virtual destructors for polymorphic classes

## Dependencies

- **LLVM**: Version 16+ (currently tested with 20.1.8)
- **CMake**: 3.16+
- **Compiler**: C++17 compatible (GCC/Clang)
- **LLVM Components**: support, core, irreader

## Current Capabilities & Limitations

**Supports:**
- Basic arithmetic with correct precedence
- Parenthesized expressions
- Floating-point numbers (double precision)
- LLVM IR generation

**Limitations:**
- Variables declared but not implemented (no symbol table)
- No error recovery in parser
- No optimization passes
- No actual execution (IR generation only)
- Limited error messages

## Testing Strategy

While no test framework exists, test expressions manually:
```bash
./build/arithc "2 + 3"        # Should output: ret double 5.000000e+00
./build/arithc "2 * 3 + 4"    # Should output: ret double 1.000000e+01
./build/arithc "(2 + 3) * 4"  # Should output: ret double 2.000000e+01
```