# ArithLang Function and Closure System Implementation Plan

## Overview

This plan implements first-class functions and lexical closures in ArithLang, integrating seamlessly with the existing immutable-by-default variable system, comprehensive error reporting, and LLVM-based code generation pipeline.

**Goals:**
- First-class functions with `fn(params) => expression` and `fn(params) { block }` syntax
- Lexical closures with immutable captures by default
- Explicit mutable captures using `mut(var1, var2)` syntax
- Complete integration with existing AST, type checking, and code generation
- Professional error reporting with source location tracking
- Comprehensive test coverage following existing patterns

## Types

### Core Function Types

```cpp
// Function parameter representation
struct FunctionParameter {
    std::string name;
    bool is_mutable;
    SourceLocation location;
    
    FunctionParameter(std::string n, bool mut, SourceLocation loc)
        : name(std::move(n)), is_mutable(mut), location(std::move(loc)) {}
};

// Captured variable representation
struct CapturedVariable {
    std::string name;
    bool is_mutable_capture;
    SourceLocation capture_location;
    
    CapturedVariable(std::string n, bool mut, SourceLocation loc)
        : name(std::move(n)), is_mutable_capture(mut), capture_location(std::move(loc)) {}
};

// Runtime closure representation
struct ClosureContext {
    // Immutable captures (copied values)
    std::vector<double> immutable_captures;
    
    // Mutable captures (shared references)
    std::vector<std::shared_ptr<double>> mutable_captures;
    
    // Mapping from capture names to indices
    std::map<std::string, size_t> immutable_indices;
    std::map<std::string, size_t> mutable_indices;
};

// LLVM function wrapper with closure context
struct FunctionValue {
    llvm::Function* llvm_function;
    std::unique_ptr<ClosureContext> closure_context;
    
    FunctionValue(llvm::Function* func, std::unique_ptr<ClosureContext> ctx)
        : llvm_function(func), closure_context(std::move(ctx)) {}
};
```

## Files

### New Files to Create

1. **`include/function_ast.h`** - Function-specific AST node declarations
2. **`src/function_codegen.cpp`** - Function code generation implementation
3. **`tests/test_function_parser.cpp`** - Function parsing unit tests
4. **`tests/test_function_codegen.cpp`** - Function code generation unit tests
5. **`tests/test_closures.cpp`** - Closure functionality unit tests
6. **`tests/k/function_*.k`** - Integration test files for functions

### Files to Modify

1. **`include/lexer.h`** - Add function-related tokens
2. **`src/lexer.cpp`** - Implement function token recognition
3. **`include/ast.h`** - Add function AST node declarations
4. **`include/parser.h`** - Add function parsing method declarations
5. **`src/parser.cpp`** - Implement function parsing logic
6. **`include/type_check.h`** - Add function type checking declarations
7. **`src/type_check.cpp`** - Implement function type validation
8. **`include/codegen.h`** - Add function code generation support
9. **`src/codegen.cpp`** - Extend with function call and closure support
10. **`CMakeLists.txt`** - Add new test executables
11. **`memory-bank/activeContext.md`** - Update with function implementation status

## Functions

### New Functions to Implement

#### Lexer Extensions (`src/lexer.cpp`)
```cpp
// Add to existing handleKeywordOrIdentifier()
bool Lexer::handleKeywordOrIdentifier() {
    // Add "fn" and "return" keyword recognition
    if (identifier == "fn") return createToken(TOK_FN);
    if (identifier == "return") return createToken(TOK_RETURN);
    // ... existing logic
}
```

#### Parser Extensions (`src/parser.cpp`)
```cpp
// New parsing methods
std::unique_ptr<ExprAST> Parser::parseFunctionLiteral();
std::unique_ptr<ExprAST> Parser::parseFunctionCall(std::unique_ptr<ExprAST> function);
std::unique_ptr<ASTNode> Parser::parseReturnStatement();
std::vector<FunctionParameter> Parser::parseParameterList();
std::vector<CapturedVariable> Parser::parseCaptureClause();

// Modified methods
std::unique_ptr<ExprAST> Parser::parsePrimary(); // Add function literal support
std::unique_ptr<ExprAST> Parser::parsePostfixExpr(); // Add function call support
std::unique_ptr<ASTNode> Parser::parseStatement(); // Add return statement support
```

#### Type Checker Extensions (`src/type_check.cpp`)
```cpp
// New validation methods
void TypeChecker::validateFunctionLiteral(FunctionLiteralAST* func);
void TypeChecker::validateFunctionCall(FunctionCallAST* call);
void TypeChecker::validateReturnStatement(ReturnStmtAST* ret);
void TypeChecker::validateCapturedVariables(const std::vector<CapturedVariable>& captures);
```

#### Code Generator Extensions (`src/codegen.cpp`)
```cpp
// New code generation methods
llvm::Value* CodeGen::generateFunctionLiteral(FunctionLiteralAST* func);
llvm::Value* CodeGen::generateFunctionCall(FunctionCallAST* call);
llvm::Value* CodeGen::generateReturnStatement(ReturnStmtAST* ret);
llvm::Function* CodeGen::createClosureFunction(FunctionLiteralAST* func);
llvm::Value* CodeGen::createClosureContext(const std::vector<CapturedVariable>& captures);
```

### Modified Functions

#### `Parser::parsePrimary()` - Add function literal parsing
```cpp
std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken.type) {
        case TOK_FN:
            return parseFunctionLiteral();
        // ... existing cases
    }
}
```

#### `TypeChecker::typeCheck()` - Add function type validation
```cpp
void TypeChecker::typeCheck(ASTNode* node) {
    if (auto* func = dynamic_cast<FunctionLiteralAST*>(node)) {
        validateFunctionLiteral(func);
    } else if (auto* call = dynamic_cast<FunctionCallAST*>(node)) {
        validateFunctionCall(call);
    }
    // ... existing type checks
}
```

## Classes

### New AST Node Classes

#### `FunctionLiteralAST : public ExprAST`
```cpp
class FunctionLiteralAST : public ExprAST {
private:
    std::vector<FunctionParameter> parameters;
    std::vector<CapturedVariable> captures;
    std::unique_ptr<ASTNode> body;
    bool is_expression_function;
    SourceLocation fn_location;
    
public:
    FunctionLiteralAST(std::vector<FunctionParameter> params,
                      std::vector<CapturedVariable> caps,
                      std::unique_ptr<ASTNode> body,
                      bool is_expr_func,
                      SourceLocation loc);
    
    llvm::Value* codegen() override;
    const std::vector<FunctionParameter>& getParameters() const;
    const std::vector<CapturedVariable>& getCaptures() const;
    ASTNode* getBody() const;
    bool isExpressionFunction() const;
    const SourceLocation& getFnLocation() const;
};
```

#### `FunctionCallAST : public ExprAST`
```cpp
class FunctionCallAST : public ExprAST {
private:
    std::unique_ptr<ExprAST> function;
    std::vector<std::unique_ptr<ExprAST>> arguments;
    SourceLocation call_location;
    
public:
    FunctionCallAST(std::unique_ptr<ExprAST> func,
                   std::vector<std::unique_ptr<ExprAST>> args,
                   SourceLocation loc);
    
    llvm::Value* codegen() override;
    ExprAST* getFunction() const;
    const std::vector<std::unique_ptr<ExprAST>>& getArguments() const;
    const SourceLocation& getCallLocation() const;
};
```

#### `ReturnStmtAST : public ASTNode`
```cpp
class ReturnStmtAST : public ASTNode {
private:
    std::unique_ptr<ExprAST> value;
    SourceLocation return_location;
    
public:
    ReturnStmtAST(std::unique_ptr<ExprAST> val, SourceLocation loc);
    ReturnStmtAST(SourceLocation loc); // For return without value
    
    llvm::Value* codegen() override;
    ExprAST* getValue() const;
    const SourceLocation& getReturnLocation() const;
};
```

### Extended Classes

#### `CodeGen` - Function Management Extensions
```cpp
class CodeGen {
private:
    // Add to existing private members
    std::map<std::string, std::unique_ptr<FunctionValue>> function_table;
    llvm::Function* current_function;
    bool in_function_context;
    
    // New helper methods
    llvm::StructType* createClosureStructType(const std::vector<CapturedVariable>& captures);
    llvm::Value* allocateClosureStruct(llvm::StructType* struct_type, 
                                      const std::vector<CapturedVariable>& captures);
    llvm::Function* generateClosureFunction(FunctionLiteralAST* func);
    
public:
    // New public interface
    void enterFunction(llvm::Function* func);
    void exitFunction();
    llvm::Function* getCurrentFunction();
    bool isInFunctionContext() const;
};
```

## Dependencies

### New Token Types (Add to `TokenType` enum)
```cpp
enum TokenType {
    // ... existing tokens
    TOK_FN = -14,        // fn keyword
    TOK_RETURN = -15,    // return keyword  
    TOK_ARROW = -16,     // => operator
    // ... existing tokens
};
```

### LLVM Dependencies
- **Function Types**: `llvm::FunctionType` for function signatures
- **Function Creation**: `llvm::Function::Create()` for closure functions
- **Struct Types**: `llvm::StructType` for closure context
- **Call Instructions**: `llvm::CallInst` for function calls
- **Return Instructions**: `llvm::ReturnInst` for return statements

### Shared Pointer Integration
- **Memory Management**: `std::shared_ptr<double>` for mutable captures
- **Reference Counting**: Automatic cleanup of shared mutable variables
- **Thread Safety**: Consider `std::atomic<double>` for future thread support

## Testing

### Unit Test Strategy

#### Function Parser Tests (`tests/test_function_parser.cpp`)
```cpp
TEST(FunctionParserTest, BasicFunctionLiteral) {
    // Test: fn(x, y) => x + y
}

TEST(FunctionParserTest, BlockFunction) {
    // Test: fn(x) { return x * 2; }
}

TEST(FunctionParserTest, MutableParameters) {
    // Test: fn(mut x, y) => x = x + y
}

TEST(FunctionParserTest, CaptureClause) {
    // Test: fn() mut(counter) { counter = counter + 1; }
}

TEST(FunctionParserTest, FunctionCall) {
    // Test: func(arg1, arg2)
}
```

#### Function CodeGen Tests (`tests/test_function_codegen.cpp`)
```cpp
TEST(FunctionCodeGenTest, SimpleFunctionCall) {
    // Test LLVM IR generation for basic function calls
}

TEST(FunctionCodeGenTest, ClosureGeneration) {
    // Test closure context creation and function generation
}

TEST(FunctionCodeGenTest, CaptureVariable) {
    // Test variable capture in closure context
}
```

#### Closure Tests (`tests/test_closures.cpp`)
```cpp
TEST(ClosureTest, ImmutableCapture) {
    // Test immutable variable capture behavior
}

TEST(ClosureTest, MutableCapture) {
    // Test mutable variable capture and modification
}

TEST(ClosureTest, NestedClosures) {
    // Test closures within closures
}
```

### Integration Test Files

#### `tests/k/function_basic.k`
```k
// EXPECTED: 42.000000000000000
add = fn(x, y) => x + y;
result = add(20, 22);
print result;
```

#### `tests/k/function_closure.k`
```k
// EXPECTED: 50.000000000000000
multiplier = 10;
scale = fn(x) => x * multiplier;
result = scale(5);
print result;
```

#### `tests/k/function_mutable_capture.k`
```k
// EXPECTED: 1.000000000000000
// EXPECTED: 2.000000000000000
mut counter = 0;
increment = fn() mut(counter) {
    counter = counter + 1;
    return counter;
};
print increment();
print increment();
```

#### `tests/k/function_higher_order.k`
```k
// EXPECTED: 25.000000000000000
apply = fn(f, x) => f(x);
square = fn(x) => x * x;
result = apply(square, 5);
print result;
```

### Error Testing Strategy

#### Negative Test Cases
- Invalid function syntax: `fn(x,) => x` (trailing comma)
- Undefined capture: `fn() mut(undefined_var) => undefined_var`
- Call non-function: `x = 42; x(10);`
- Wrong argument count: `add = fn(x, y) => x + y; add(1);`
- Return outside function: `return 42;` at top level

## Implementation Order

### Phase 1: Core Function Infrastructure (Week 1)
1. **Lexer Extensions**
   - Add `TOK_FN`, `TOK_RETURN`, `TOK_ARROW` tokens
   - Implement keyword recognition in `handleKeywordOrIdentifier()`
   - Add operator recognition for `=>` in `handleOperator()`

2. **AST Node Implementation**
   - Create `FunctionLiteralAST`, `FunctionCallAST`, `ReturnStmtAST` classes
   - Add to `include/ast.h` with proper source location tracking
   - Implement basic getter methods and constructors

3. **Basic Parser Implementation**
   - Implement `parseFunctionLiteral()` for simple expression functions
   - Add function call parsing in `parsePostfixExpr()`
   - Handle `return` statements in `parseStatement()`
   - Add comprehensive error handling with location tracking

### Phase 2: Type Checking and Validation (Week 1-2)
4. **Type Checker Extensions**
   - Implement `validateFunctionLiteral()` with parameter validation
   - Add `validateFunctionCall()` with argument count checking
   - Validate capture variable existence and mutability
   - Integrate with existing type checking pipeline

5. **Parser Completion**
   - Implement parameter list parsing with mutability markers
   - Add capture clause parsing `mut(var1, var2)`
   - Handle block functions with proper scope management
   - Add precedence handling for function calls

### Phase 3: LLVM Code Generation (Week 2)
6. **Basic Code Generation**
   - Implement closure context struct generation
   - Create LLVM functions for function literals
   - Generate function call instructions
   - Handle return statement code generation

7. **Closure Implementation**
   - Implement immutable capture (value copying)
   - Add mutable capture with shared pointers
   - Create closure context allocation and initialization
   - Handle variable lookup in closure contexts

### Phase 4: Integration and Testing (Week 2-3)
8. **Symbol Table Integration**
   - Extend `CodeGen` scope management for functions
   - Handle variable binding in function contexts
   - Implement function value storage and retrieval

9. **Comprehensive Testing**
   - Create all unit tests following existing patterns
   - Add integration test `.k` files with `EXPECTED:` comments
   - Test error cases with proper location reporting
   - Validate memory management and cleanup

### Phase 5: Advanced Features and Optimization (Week 3)
10. **Advanced Closure Features**
    - Implement nested closures
    - Add higher-order function support
    - Handle complex capture scenarios
    - Optimize closure context layout

11. **Error Reporting Polish**
    - Add function-specific error messages
    - Implement helpful suggestions for common mistakes
    - Ensure all errors follow GCC-style format
    - Add context-aware error reporting

### Phase 6: Documentation and Finalization (Week 3-4)
12. **Documentation Updates**
    - Update `memory-bank/activeContext.md` with implementation status
    - Add function examples to language documentation
    - Create usage patterns and best practices guide

13. **Final Integration**
    - Update `CMakeLists.txt` with new test targets
    - Ensure all tests pass in CI/CD pipeline
    - Performance testing and optimization
    - Code review and cleanup

## Integration Points

### With Variable System
- Function bindings follow immutability rules (`add = fn(x) => x` is immutable)
- Mutable function variables: `mut handler = fn(x) => x; handler = different_func;`
- Captured variables respect original mutability declarations
- `mut(var)` capture syntax validates `var` was declared mutable

### With Type System
- Functions are values of "function type" (implementation detail)
- Type checking validates parameter count and usage
- Return type inference from function body
- Future: Explicit type annotations `add: (double, double) -> double`

### With Error Reporting
- All function-related errors use existing `ParseError` with `SourceLocation`
- Function call errors show both call site and function definition locations
- Capture errors highlight both capture site and variable declaration
- Consistent with existing GCC-style error format

### With Module System (Future)
- Functions can be exported: `export add = fn(x, y) => x + y;`
- Functions can be imported: `import { add } from "math";`
- Closures capture from module scope appropriately
- Function name resolution follows module rules

This implementation plan provides a complete roadmap for adding first-class functions and closures to ArithLang while maintaining the existing architecture patterns, error reporting quality, and testing standards. The phased approach ensures each component is properly tested before building the next layer of functionality.
