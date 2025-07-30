# Technical Context: ArithLang

## Technology Stack

### Core Technologies
- **C++17**: Modern C++ with smart pointers, auto typing, and standard library features
- **LLVM**: Industry-standard compiler infrastructure for IR generation and optimization
- **CMake**: Cross-platform build system management
- **Google Test**: Unit testing framework for comprehensive test coverage

### Development Environment
- **Compiler**: Requires C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **LLVM Version**: Compatible with LLVM 10+ (tested configurations)
- **Build System**: CMake 3.16 minimum
- **Platform**: Cross-platform (Linux, macOS, Windows)

## Project Structure

### Source Organization
```
include/          # Header files
├── ast.h         # AST node definitions
├── codegen.h     # LLVM code generation
├── lexer.h       # Tokenization
└── parser.h      # Syntax analysis

src/              # Implementation files
├── codegen.cpp   # LLVM integration
├── lexer.cpp     # Tokenizer implementation
├── main.cpp      # CLI interface
└── parser.cpp    # Parser implementation

tests/            # Test suite
├── test_lexer.cpp      # Lexer unit tests
├── test_parser.cpp     # Parser unit tests
├── test_codegen.cpp    # Code generation tests
├── test_integration.cpp # End-to-end tests
└── k/                  # Test input files
```

### Build Artifacts
```
build/            # CMake build directory
├── arithc        # Main compiler executable
├── test_*        # Individual test executables
└── k/            # Generated LLVM IR files
```

## Dependencies

### Required
- **LLVM Development Libraries**: Core requirement for IR generation
- **CMake**: Build system
- **C++17 Standard Library**: STL containers, smart pointers, etc.

### Bundled (via CMake FetchContent)
- **Google Test**: Testing framework, automatically downloaded during build

## Build Process

### Initial Build (from project root)
```bash
mkdir -p build
cd build
cmake ..
make
```

### Rebuild (from project root)
```bash
cmake --build build
```

### Build Targets
- `arithc`: Main compiler executable
- `test_lexer`: Lexer unit tests
- `test_parser`: Parser unit tests
- `test_codegen`: Code generation tests
- `test_integration`: End-to-end tests

## Test Execution

### Complete Test Sequence
Follow this standardized testing sequence:

1. **Build**: Use initial build or rebuild process above

2. **Unit Tests**: Navigate to build directory and run comprehensive unit tests
   ```bash
   cd build
   ctest
   ```

3. **Integration Tests**: Return to project root and run system tests
   ```bash
   cd ..
   ./test_runner.sh
   ```

### Individual Test Execution
```bash
# Individual unit test suites (from build directory)
./test_lexer
./test_parser
./test_codegen
./test_integration
```

## LLVM Integration Details

### LLVM Components Used
- **Core**: Basic LLVM types and values
- **IRBuilder**: Instruction generation
- **Module**: Code container
- **ExecutionEngine**: JIT compilation and execution
- **Support**: Error handling and utilities

### Code Generation Strategy
- **Target**: Generic LLVM IR (platform-independent)
- **Function Model**: Single main function containing all statements
- **Type System**: Primarily double-precision floating point
- **Memory Model**: Stack-allocated variables via alloca

## Testing Strategy

### Unit Testing
- **Lexer Tests**: Token generation, error handling
- **Parser Tests**: AST construction, syntax validation
- **Codegen Tests**: LLVM IR generation correctness

### Integration Testing
- **End-to-End**: Complete source → execution pipeline
- **File-based**: Test programs in `tests/k/` directory
- **Output Validation**: Compare expected vs. actual results

## Development Workflow

### Adding Features
1. Write failing tests first
2. Implement lexer changes (if needed)
3. Update parser for new syntax
4. Extend AST nodes
5. Implement code generation
6. Validate all tests pass

### Debugging
- **LLVM IR Inspection**: Generated `.ll` files in `build/k/`
- **Verbose Output**: Compiler can output intermediate representations
- **Test Isolation**: Individual test executables for focused debugging

## Performance Considerations

### Compilation Speed
- Single-pass compilation
- Minimal optimization focus (educational priority)
- Direct LLVM IR generation

### Runtime Performance
- LLVM JIT compilation for immediate execution
- No custom optimization passes
- Standard LLVM optimization available if needed

## Known Limitations

### Technical Constraints
- **Single Scope**: No nested scoping or functions
- **Type System**: Only double precision floating point
- **Memory Management**: Simple stack allocation only
- **Error Recovery**: Limited error recovery in parser

### LLVM Dependencies
- Requires LLVM development headers
- Platform-specific LLVM installation requirements
- Version compatibility considerations
