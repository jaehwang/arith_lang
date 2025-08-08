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

rules/            # Project documentation standards
└── commit_log.md # Standardized commit message template
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

### Compiler Usage
- **Basic Usage**: `./arithc input.k` → creates `a.ll` in current directory
- **Custom Output**: `./arithc -o output.ll input.k` → creates specified output file
- **gcc Compatibility**: Follows familiar gcc-style default behavior

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

## Development Standards

### Commit Message Standards
All commits must follow the standardized template defined in `rules/commit_log.md`.

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

## Unit Test Creation Rules

### Comprehensive Test Coverage Requirements
모든 새로운 기능과 컴포넌트에 대해 **정상 입력과 비정상 입력 모두**를 고려한 테스트 케이스를 작성해야 합니다.

### Positive Test Cases (정상 케이스)
각 기능에 대해 다음 정상 시나리오를 테스트:
- **기본 기능**: 표준적인 사용 패턴과 예상되는 올바른 입력
- **경계 조건**: 유효한 범위의 최소값/최대값 (예: 빈 프로그램, 단일 문장)
- **복합 시나리오**: 여러 기능이 조합된 복잡한 올바른 입력
- **특수 케이스**: Edge case이지만 유효한 입력 (예: 중첩된 괄호, 긴 변수명)

### Negative Test Cases (비정상 케이스)
각 기능에 대해 다음 오류 시나리오를 **반드시** 테스트:
- **구문 오류**: 잘못된 문법, 누락된 구문 요소 (예: 세미콜론, 괄호)
- **의미적 오류**: 문법적으로는 올바르나 의미적으로 잘못된 입력
- **타입 오류**: 지원하지 않는 데이터 타입이나 형변환 오류
- **범위 오류**: 허용 범위를 벗어난 입력값
- **빈 입력**: null, empty string 등의 빈 입력 처리

### Test Structure Pattern
```cpp
// Google Test 기반 테스트 구조
class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override { /* 테스트 환경 초기화 */ }
    void TearDown() override { /* 정리 작업 */ }
};

// 정상 케이스 테스트
TEST_F(ComponentTest, ValidInput_BasicCase) {
    // Given: 올바른 입력 준비
    // When: 기능 실행  
    // Then: 예상 결과 검증
    EXPECT_EQ(expected, actual);
}

// 비정상 케이스 테스트
TEST_F(ComponentTest, InvalidInput_ThrowsException) {
    // Given: 잘못된 입력 준비
    // When & Then: 예외 발생 검증
    EXPECT_THROW(function_call(invalid_input), std::runtime_error);
}
```

### Error Testing Guidelines
- **EXPECT_THROW**: 예외 발생 여부와 예외 타입 검증
- **Error Message Validation**: 예외 메시지의 정확성과 유용성 검증
- **Error Recovery**: 오류 후 시스템 상태 일관성 검증
- **Multiple Errors**: 한 번에 여러 오류가 발생하는 경우 처리 검증

### Test Organization
- **Separate Test Classes**: 정상 케이스와 비정상 케이스를 별도 테스트 클래스로 분리 고려
- **Parametrized Tests**: 동일한 로직의 다양한 입력에 대해 `TEST_P` 사용
- **Test Naming**: `ComponentTest_Scenario_ExpectedResult` 패턴 사용
- **Test Documentation**: 복잡한 테스트 케이스에 대한 주석과 문서화

### Regression Prevention
- **Bug-Driven Tests**: 발견된 버그마다 해당 시나리오의 테스트 케이스 추가
- **Syntax Error Coverage**: 파서의 모든 구문 오류 경로에 대한 테스트
- **Edge Case Documentation**: 특이한 케이스와 그 처리 방법 문서화

## Development Workflow

### Adding Features
1. Write failing tests first
2. Implement lexer changes (if needed)
3. Update parser for new syntax
4. Extend AST nodes
5. Implement code generation
6. Validate all tests pass
7. Commit following `rules/commit_log.md` template

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

### Output Precision Specification
- **Print Precision**: Fixed at 15 decimal places using "%.15f" printf format
- **Rationale**: Matches IEEE 754 double precision effective digits (~15-17)
- **Scientific Accuracy**: Enables precise display of mathematical calculations (e.g., Pi)
- **Trade-off**: Fixed precision vs. configurable precision complexity

### LLVM Dependencies
- Requires LLVM development headers
- Platform-specific LLVM installation requirements
- Version compatibility considerations
