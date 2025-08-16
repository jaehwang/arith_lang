# ArithLang Closure Specification

## Overview

This specification defines closure support for ArithLang, enabling higher-order functions with lexical scoping and variable capture. Closures allow functions to capture variables from their surrounding scope and use them even after the enclosing scope has ended.

## Syntax

### Function Literals (Lambda Expressions)

```bnf
<function_literal> ::= "fn" "(" <parameter_list>? ")" <block>
                     | "fn" "(" <parameter_list>? ")" "=>" <expression>

<parameter_list>   ::= <identifier> ("," <identifier>)*
```

### Function Calls

```bnf
<function_call>    ::= <primary> "(" <argument_list>? ")"

<argument_list>    ::= <expression> ("," <expression>)*
```

### Updated Grammar Integration

The existing grammar needs these updates:

```bnf
<primary>      ::= <number>
                 | <identifier>
                 | <string_literal>
                 | <function_literal>
                 | <function_call>
                 | "(" <expression> ")"

<assignment>   ::= <identifier> "=" <expression>
                 | <identifier> "=" <function_literal>
```

## Language Features

### 1. Function Literals

#### Block Form
```k
// Multi-statement function
add = fn(x, y) {
    result = x + y;
    print "Computing: ", x, " + ", y, " = ", result;
    return result;
};
```

#### Expression Form (Arrow Functions)
```k
// Single expression function
square = fn(x) => x * x;
isPositive = fn(n) => n > 0;
```

### 2. Variable Capture (Closures)

#### Capturing by Value
```k
// Outer variable captured by value
multiplier = 10;
scale = fn(x) => x * multiplier;

result = scale(5);  // result = 50
print result;
```

#### Capturing Mutable State
```k
// Counter closure with mutable capture
counter = fn() {
    count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
};

myCounter = counter();
print myCounter();  // 1
print myCounter();  // 2
print myCounter();  // 3
```

### 3. Higher-Order Functions

#### Functions as Parameters
```k
// Apply function to value
apply = fn(f, x) => f(x);

double = fn(x) => x * 2;
result = apply(double, 21);  // result = 42
```

#### Functions as Return Values
```k
// Function factory
makeAdder = fn(n) => fn(x) => x + n;

add5 = makeAdder(5);
add10 = makeAdder(10);

print add5(3);   // 8
print add10(7);  // 17
```

## Semantics

### 1. Lexical Scoping

Variables are resolved according to lexical (static) scoping rules:

1. **Local parameters** have highest precedence
2. **Captured variables** from enclosing scopes
3. **Global variables** as fallback

```k
x = 100;  // global

outer = fn() {
    x = 10;  // outer scope
    
    inner = fn(x) {  // parameter shadows outer x
        return x * 2;  // uses parameter x
    };
    
    return inner;
};

f = outer();
result = f(5);  // result = 10 (parameter x = 5)
```

### 2. Variable Capture Mechanisms

#### Capture by Value (Default)
```k
// Variables captured by value at closure creation time
values = [];
i = 0;
while (i < 3) {
    // Each closure captures current value of i
    values[i] = fn() => i;
    i = i + 1;
}
// All closures return different values: 0, 1, 2
```

#### Capture by Reference (Mutable)
```k
// Explicit mutable capture using 'mut' keyword
counter = 0;
increment = fn() mut(counter) => {
    counter = counter + 1;
    return counter;
};
// counter is modified by reference
```

### 3. Function Type System

#### Function Types
```k
// Function type notation: (param_types) -> return_type
// add: (double, double) -> double
add = fn(x, y) => x + y;

// filter: (array, (double) -> bool) -> array
filter = fn(arr, predicate) => {
    // implementation
};
```

## Implementation Approach

### 1. AST Nodes

#### New AST Node Types
```cpp
// Function literal AST
class FunctionLiteralAST : public ASTNode {
private:
    std::vector<std::string> parameters;
    std::unique_ptr<ASTNode> body;  // BlockAST or ExpressionAST
    std::vector<CapturedVariable> captures;
    
public:
    FunctionLiteralAST(std::vector<std::string> params, 
                      std::unique_ptr<ASTNode> body);
    Value* codegen() override;
};

// Function call AST
class FunctionCallAST : public ASTNode {
private:
    std::unique_ptr<ASTNode> function;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    
public:
    FunctionCallAST(std::unique_ptr<ASTNode> func,
                   std::vector<std::unique_ptr<ASTNode>> args);
    Value* codegen() override;
};

// Return statement AST
class ReturnStmtAST : public ASTNode {
private:
    std::unique_ptr<ASTNode> value;
    
public:
    ReturnStmtAST(std::unique_ptr<ASTNode> val);
    Value* codegen() override;
};
```

### 2. Lexer Extensions

#### New Tokens
```cpp
enum TokenType {
    // ... existing tokens ...
    TOK_FN,         // "fn"
    TOK_RETURN,     // "return" 
    TOK_ARROW,      // "=>"
    TOK_LPAREN,     // "("
    TOK_RPAREN,     // ")"
    TOK_COMMA,      // ","
    TOK_MUT,        // "mut" (for mutable captures)
};
```

### 3. Parser Extensions

#### Function Parsing Methods
```cpp
class Parser {
private:
    std::unique_ptr<FunctionLiteralAST> parseFunctionLiteral();
    std::vector<std::string> parseParameterList();
    std::unique_ptr<FunctionCallAST> parseFunctionCall(std::unique_ptr<ASTNode> func);
    std::vector<std::unique_ptr<ASTNode>> parseArgumentList();
    std::unique_ptr<ReturnStmtAST> parseReturnStatement();
};
```

### 4. LLVM Code Generation

#### Closure Implementation Strategy

**Option 1: Function Pointers + Capture Struct**
```cpp
// Closure represented as struct containing:
struct Closure {
    void (*function_ptr)(void* captures, args...);
    void* captured_variables;  // Heap-allocated capture struct
};
```

**Option 2: LLVM Function Objects**
```cpp
// Generate LLVM functions with closure context parameter
// Original: fn(x, y) => x + y + captured_z
// Generated: double closure_fn(double* captures, double x, double y)
```

#### Memory Management
```cpp
class ClosureCodeGen {
private:
    // Analyze which variables are captured
    std::vector<CapturedVariable> analyzeCaptures(FunctionLiteralAST* func);
    
    // Generate capture struct type
    StructType* createCaptureStruct(const std::vector<CapturedVariable>& captures);
    
    // Generate closure allocation and initialization
    Value* createClosure(FunctionLiteralAST* func);
    
    // Generate function call with closure context
    Value* callClosure(Value* closure, const std::vector<Value*>& args);
};
```

### 5. Scope and Symbol Management

#### Enhanced Symbol Table
```cpp
class SymbolTable {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    std::vector<CapturedVariable> current_captures;
    
public:
    void enterScope();
    void exitScope();
    void addCapture(const std::string& name, Type type);
    std::vector<CapturedVariable> getCurrentCaptures();
};

struct CapturedVariable {
    std::string name;
    Type type;
    bool is_mutable;
    int scope_level;
};
```

## Test Cases

### 1. Basic Function Literals
```k
// TEST: basic_function_literal.k
// EXPECTED: 42.000000000000000
add = fn(x, y) => x + y;
result = add(20, 22);
print result;
```

### 2. Variable Capture
```k
// TEST: variable_capture.k
// EXPECTED: 150.000000000000000
multiplier = 10;
scale = fn(x) => x * multiplier;
result = scale(15);
print result;
```

### 3. Nested Closures
```k
// TEST: nested_closures.k
// EXPECTED: 8.000000000000000
makeAdder = fn(n) => fn(x) => x + n;
add5 = makeAdder(5);
result = add5(3);
print result;
```

### 4. Mutable Captures
```k
// TEST: mutable_captures.k
// EXPECTED: 1.000000000000000
// EXPECTED: 2.000000000000000
// EXPECTED: 3.000000000000000
count = 0;
increment = fn() mut(count) => {
    count = count + 1;
    return count;
};

print increment();
print increment(); 
print increment();
```

### 5. Higher-Order Functions
```k
// TEST: higher_order.k
// EXPECTED: 4.000000000000000
// EXPECTED: 16.000000000000000
apply = fn(f, x) => f(x);
square = fn(x) => x * x;

print apply(square, 2);
print apply(square, 4);
```

## Error Handling

### 1. Parse Errors
- Missing parameter lists: `fn => x + 1`
- Invalid parameter syntax: `fn(x y) => x + y`
- Missing function body: `fn(x)`

### 2. Type Errors
- Calling non-function: `x = 5; x(10);`
- Wrong argument count: `fn(x, y) => x + y; f(1);`
- Capture of undeclared variable

### 3. Runtime Errors
- Function call on null closure
- Stack overflow from infinite recursion

## Integration with Existing Features

### 1. Print Statements
```k
// Functions work with existing print functionality
f = fn(x) => x * 2;
print "Result: %.2f", f(3.5);  // Result: 7.00
```

### 2. Control Flow
```k
// Functions in conditional expressions
isEven = fn(x) => (x % 2) == 0;
if (isEven(4)) {
    print "4 is even";
} else {
    print "4 is odd";
}
```

### 3. While Loops
```k
// Functions with loop constructs
condition = fn() => count < 10;
count = 0;
while (condition()) {
    print count;
    count = count + 1;
}
```

## Implementation Phases

### Phase 1: Basic Function Literals
- Function literal parsing and AST
- Simple function calls without captures
- Return statements
- Basic code generation

### Phase 2: Variable Capture
- Scope analysis for captures
- Capture struct generation
- Closure creation and calling

### Phase 3: Advanced Features
- Mutable captures
- Nested closures
- Higher-order function patterns

### Phase 4: Optimization
- Escape analysis for stack vs heap allocation
- Inlining of simple closures
- Dead capture elimination

## Memory Model

### 1. Closure Lifetime Management
```cpp
// Closures are heap-allocated with reference counting
struct Closure {
    int ref_count;
    FunctionType* function;
    void* captures;
};

void retain_closure(Closure* closure);
void release_closure(Closure* closure);
```

### 2. Capture Storage
- **By-value captures**: Copied into closure capture struct
- **By-reference captures**: Pointer to shared memory location
- **Automatic memory management**: Reference counting for shared captures

This specification provides a comprehensive foundation for implementing closures in ArithLang while maintaining compatibility with the existing language design and LLVM-based code generation approach.