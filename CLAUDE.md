# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ArithLang is an LLVM-based arithmetic expression compiler written in C++17. It follows a classic four-stage compiler pipeline: Lexer → Parser → AST → CodeGen, transforming mathematical expressions into executable LLVM IR.

## Build Commands

```bash
# Standard build process
mkdir -p build && cd build && cmake .. && make

# Rebuild after changes
cmake --build build

# Clean rebuild
rm -rf build && mkdir build && cd build && cmake .. && make

# Run tests
cd build && make test_syntax && ./test_syntax
```

## Usage

ArithLang now uses file-based input/output with `.k` source files:

```bash
# Command line format
./build/arithc -o <output_file> <input_file>

# Create source files (.k extension required)
echo "x = 42; y = x * 2 + 10; print y;" > example.k

# Compile to LLVM IR
./build/arithc -o output.ll example.k

# Execute generated IR
lli output.ll

# Show help
./build/arithc
```

## Architecture Overview

The codebase implements a classic compiler pipeline with clear separation of concerns:

### 1. Lexer (`lexer.h/cpp`)
- Character-by-character tokenization with lookahead
- Supports: floating-point numbers, identifiers, operators (`+`, `-`, `*`, `/`, `>`, `<`, `>=`, `<=`, `==`, `!=`), parentheses
- Multi-character operator recognition (`>=`, `<=`, `==`, `!=`)
- Keywords: `if`, `else`, `while`, `print`
- Token structure: `{type, value, numValue}` with enum-based types

### 2. Parser (`parser.h/cpp`)
- Operator precedence parsing (Pratt parser style)
- Precedence levels: comparison operators (5), `+-` (10), `*/` (40)
- Recursive descent with left-associative binary operators
- Control flow parsing: if-else statements, while loops, blocks
- Error handling via exceptions

### 3. AST (`ast.h`)
- Expression nodes: `NumberExprAST`, `VariableExprAST`, `BinaryExprAST`, `AssignmentExprAST`
- Statement nodes: `PrintStmtAST`, `IfStmtAST`, `WhileStmtAST`, `BlockAST`
- Base classes: `ASTNode` (all nodes), `ExprAST` (expressions only)
- Virtual `codegen()` method for visitor pattern
- Memory management via `std::unique_ptr`

### 4. CodeGen (`codegen.h/cpp`)
- LLVM IR generation using IRBuilder
- Singleton pattern with global LLVM context/module management
- Variable storage using alloca/load/store for mutable variables
- Floating-point arithmetic and comparison instructions
- Control flow with basic blocks and conditional branches
- Printf integration for output statements

### 5. Main Driver (`main.cpp`)
- File-based command-line interface with `-o` option
- Reads `.k` source files and writes LLVM IR to specified output file
- Creates LLVM function returning double
- Includes Korean help messages and error handling

## Development Guidelines

### Variable Storage Architecture
ArithLang uses LLVM's alloca/load/store model for mutable variables:
- Variables are allocated on the stack using `alloca` instructions in function entry block
- Variable access uses `load` instructions to read values
- Variable assignment uses `store` instructions to write values
- This enables proper variable mutation in loops and control flow

### Adding New Operators
1. Add token type to `TokenType` enum in `lexer.h`
2. Update lexer's `getNextToken()` to recognize the operator (handle multi-character ops)
3. Add precedence entry in `Parser` constructor (`std::map<int, int>`)
4. Implement operator logic in `BinaryExprAST::codegen()`
   - Arithmetic: use `CreateFAdd`, `CreateFSub`, etc.
   - Comparison: use `CreateFCmpOLT`, `CreateFCmpOEQ`, etc. + `CreateUIToFP` for bool-to-double

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
- **함수, 메소드는 40 줄 이하로 작성하자.**

## Dependencies

- **LLVM**: Version 16+ (currently tested with 20.1.8)
- **CMake**: 3.16+
- **Compiler**: C++17 compatible (GCC/Clang)
- **LLVM Components**: support, core, irreader

## Current Capabilities & Limitations

**Supports:**
- Arithmetic operations with correct precedence (`+`, `-`, `*`, `/`)
- Comparison operations (`>`, `<`, `>=`, `<=`, `==`, `!=`)
- Parenthesized expressions
- Floating-point numbers (double precision)
- Variable assignment and modification (`x = 1`, `x = x + 1`)
- Print statements (`print x`)
- Conditional statements (`if-else`)
- While loops with proper variable scoping
- Block statements with multiple statements
- Line comments (`// comment text`)
- LLVM IR generation with source filename tracking
- IR execution via LLVM interpreter (`lli`)

**Limitations:**
- No error recovery in parser
- No optimization passes
- Limited error messages
- Non-standard main function signature (returns double instead of int)

## Testing Strategy

### Automated Integration Tests

The project includes an automated test runner for `.k` source files:

```bash
# Run all integration tests
./test_runner.sh
```

The test runner:
- Compiles each `.k` file in `tests/k/` directory
- Executes the generated LLVM IR using `lli`
- Compares output with expected results specified in `// EXPECTED:` comments
- Provides colored pass/fail output with test statistics

**Test File Format:**
```k
// EXPECTED: 120.000000
n = 5;
result = 1;
while (n > 0) {
    result = result * n;
    n = n - 1;
}
print result;
```

### Unit Tests

The project uses Google Test for unit testing. Test files are in the `tests/` directory:

```bash
# Run all unit tests
cd build && make test_syntax && ./test_syntax

# Run specific test using CMake's test runner
cd build && ctest
```

### Manual Testing

Manual testing of language features:
```bash
# Basic arithmetic
echo "2 + 3 * 4" > basic.k
./build/arithc -o basic.ll basic.k && lli basic.ll

# Variables and assignment
echo "x = 42; y = x * 2; print y;" > vars.k
./build/arithc -o vars.ll vars.k && lli vars.ll

# Comparison operators
echo "x = 5; y = 3; if (x > y) { print 1; } else { print 0; }" > comp.k
./build/arithc -o comp.ll comp.k && lli comp.ll

# While loops
echo "x = 3; while (x) { print x; x = x - 1; }" > loop.k
./build/arithc -o loop.ll loop.k && lli loop.ll

# Complex example (factorial)
echo "n = 5; result = 1; while (n > 0) { result = result * n; n = n - 1; } print result;" > factorial.k
./build/arithc -o factorial.ll factorial.k && lli factorial.ll  # Should output: 120.000000
```

## IR Execution

Generated LLVM IR can be executed directly:

```bash
# Execute IR using LLVM interpreter
echo "x=1;print x;" > test.k
./build/arithc -o test.ll test.k && lli test.ll

# Compile to native executable
echo "x=1;print x;" > test.k
./build/arithc -o test.ll test.k
llc test.ll -o test.s
gcc test.s -o test_exec
./test_exec
```

## Error Handling

The compiler provides Korean error messages for common issues:

```bash
# Missing arguments
./build/arithc
# Output: Usage help in Korean

# Wrong file extension
./build/arithc -o out.ll test.txt
# Output: 오류: 입력 파일은 .k 확장자를 사용해야 합니다.

# File not found
./build/arithc -o out.ll missing.k
# Output: 오류: 파일을 열 수 없습니다: missing.k
```

## Personal Memories

- 나에게 말할 때 가능하면 한국어를 사용하자.