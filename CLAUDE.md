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
- File-based command-line interface with `-o` option
- Reads `.k` source files and writes LLVM IR to specified output file
- Creates LLVM function returning double
- Includes Korean help messages and error handling

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
- **함수, 메소드는 40 줄 이하로 작성하자.**

## Dependencies

- **LLVM**: Version 16+ (currently tested with 20.1.8)
- **CMake**: 3.16+
- **Compiler**: C++17 compatible (GCC/Clang)
- **LLVM Components**: support, core, irreader

## Current Capabilities & Limitations

**Supports:**
- Basic arithmetic with correct precedence (`+`, `-`, `*`, `/`)
- Parenthesized expressions
- Floating-point numbers (double precision)
- Variable assignment (`x = 1`)
- Print statements (`print x`)
- LLVM IR generation
- IR execution via LLVM interpreter (`lli`)

**Limitations:**
- No error recovery in parser
- No optimization passes
- Limited error messages
- Non-standard main function signature (returns double instead of int)

## Testing Strategy

The project uses Google Test for unit testing. Test files are in the `tests/` directory:

```bash
# Run all tests
cd build && make test_syntax && ./test_syntax

# Run specific test using CMake's test runner
cd build && ctest
```

Manual testing of expressions:
```bash
# Create test files
echo "2 + 3" > test1.k
echo "2 * 3 + 4" > test2.k
echo "(2 + 3) * 4" > test3.k
echo "x = 1; print x;" > test4.k

# Test compilation
./build/arithc -o test1.ll test1.k  # Should generate IR with ret double 5.000000e+00
./build/arithc -o test2.ll test2.k  # Should generate IR with ret double 1.000000e+01
./build/arithc -o test3.ll test3.k  # Should generate IR with ret double 2.000000e+01
./build/arithc -o test4.ll test4.k  # Should generate IR with printf call

# Test execution
lli test4.ll  # Should output: 1.000000
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