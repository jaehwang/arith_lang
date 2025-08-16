# ArithLang Function and Closure System Specification

## Overview

ArithLang provides a comprehensive function system with **first-class functions**, **lexical closures**, and **immutable-by-default** variable capture. Functions are values that can be passed around, stored in variables, and called dynamically. The closure system integrates seamlessly with the immutable variable system, providing both safety and flexibility.

## Core Principles

1. **Functions as Values**: Functions are first-class citizens that can be assigned to variables
2. **Lexical Closures**: Functions capture variables from their lexical scope
3. **Immutable Captures**: Variables are captured as immutable by default
4. **Explicit Mutable Captures**: Use `mut()` syntax for mutable captures
5. **Type Safety**: Function signatures and captures are type-checked
6. **Memory Safety**: Automatic lifetime management for captured variables

## Syntax

### Function Literals

```bnf
<function_literal> ::= "fn" "(" <parameter_list>? ")" <capture_clause>? <function_body>

<parameter_list>   ::= <parameter> ("," <parameter>)*
<parameter>        ::= <mut_qualifier>? <identifier> (":" <type>)?

<capture_clause>   ::= "mut" "(" <capture_list> ")"
<capture_list>     ::= <identifier> ("," <identifier>)*

<function_body>    ::= "=>" <expression>
                     | <block>

<function_call>    ::= <primary> "(" <argument_list>? ")"
<argument_list>    ::= <expression> ("," <expression>)*
```

### Basic Function Syntax

```k
// Expression functions (single expression)
add = fn(x, y) => x + y;
square = fn(x) => x * x;
is_positive = fn(n) => n > 0;

// Block functions (multiple statements)
factorial = fn(n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
};

// Functions with mutable parameters
process = fn(mut x, y) {
    x = x * 2;  // OK: x is mutable
    return x + y;
};
```

## Function Types and Signatures

### 1. Basic Function Types

```k
// Function type annotation (future feature)
add: (double, double) -> double = fn(x, y) => x + y;
predicate: (double) -> bool = fn(x) => x > 0;
transformer: (double) -> double = fn(x) => x * 2;
```

### 2. Higher-Order Functions

```k
// Functions that take functions as parameters
apply = fn(f, x) => f(x);
compose = fn(f, g) => fn(x) => f(g(x));
filter = fn(predicate, list) {
    // implementation
};

// Functions that return functions
make_adder = fn(n) => fn(x) => x + n;
make_multiplier = fn(factor) {
    return fn(x) => x * factor;
};
```

## Closure System

### 1. Immutable Captures (Default)

```k
// Variables are captured by value (immutable)
multiplier = 10;
scale = fn(x) => x * multiplier;  // captures multiplier immutably

result = scale(5);  // result = 50
// multiplier is captured as 10, even if original changes
multiplier = 20;    // this doesn't affect the closure
print scale(5);     // still prints 50
```

### 2. Mutable Captures (Explicit)

```k
// Explicit mutable capture using mut() syntax
mut counter = 0;
increment = fn() mut(counter) {
    counter = counter + 1;
    return counter;
};

print increment();  // 1
print increment();  // 2
print counter;      // 2 (counter was modified by closure)
```

### 3. Multiple Captures

```k
base = 10;
mut total = 0;

// Capture both immutable and mutable variables
accumulate = fn(value) mut(total) {
    // base is captured immutably
    // total is captured mutably
    adjusted_value = value + base;
    total = total + adjusted_value;
    return total;
};

print accumulate(5);   // 15 (5 + 10 = 15, total = 15)
print accumulate(3);   // 28 (3 + 10 = 13, total = 15 + 13 = 28)
```

### 4. Nested Closures

```k
// Closures can be nested and capture from multiple levels
make_counter_factory = fn(start) {
    return fn(step) {
        mut count = start;  // captures start from outer scope
        return fn() mut(count) {
            count = count + step;  // captures step and count
            return count;
        };
    };
};

counter1 = make_counter_factory(0)(1);  // starts at 0, step by 1
counter2 = make_counter_factory(100)(5); // starts at 100, step by 5

print counter1();  // 1
print counter1();  // 2
print counter2();  // 105
print counter2();  // 110
```

## Function Parameters and Mutability

### 1. Immutable Parameters (Default)

```k
// Parameters are immutable by default
calculate = fn(x, y, z) {
    // x, y, z cannot be modified
    result = x * y + z;
    // x = x + 1;  // ERROR: cannot assign to immutable parameter
    return result;
};
```

### 2. Mutable Parameters

```k
// Explicit mutable parameters
normalize = fn(mut values, target_sum) {
    mut current_sum = sum(values);
    if (current_sum != target_sum) {
        scale_factor = target_sum / current_sum;
        values = values.map(fn(x) => x * scale_factor);
    }
    return values;
};
```

### 3. Mixed Parameter Types

```k
transform = fn(data, mut cache, config) {
    // data: immutable
    // cache: mutable (can be modified)  
    // config: immutable
    
    key = generate_key(data, config);
    if (cache.has(key)) {
        return cache.get(key);
    }
    
    result = expensive_operation(data, config);
    cache.set(key, result);  // OK: cache is mutable
    return result;
};
```

## Return Statements

### 1. Explicit Returns

```k
// Explicit return statement
factorial = fn(n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
};
```

### 2. Implicit Returns (Expression Functions)

```k
// Expression functions return the expression value
add = fn(x, y) => x + y;           // returns x + y
is_even = fn(n) => (n % 2) == 0;   // returns boolean
```

### 3. Block Functions with Implicit Returns

```k
// Last expression in block is returned if no explicit return
max = fn(a, b) {
    if (a > b) {
        a  // implicit return
    } else {
        b  // implicit return
    }
};
```

## Advanced Closure Patterns

### 1. Partial Application

```k
// Currying and partial application
add = fn(x) => fn(y) => x + y;
add_5 = add(5);
result = add_5(10);  // 15

// Multi-parameter currying
multiply = fn(a) => fn(b) => fn(c) => a * b * c;
double = multiply(2);
quad_double = double(2);
result = quad_double(5);  // 20
```

### 2. Function Factories

```k
// Create specialized functions
make_validator = fn(min, max) {
    return fn(value) => value >= min && value <= max;
};

age_validator = make_validator(0, 120);
percentage_validator = make_validator(0, 100);

print age_validator(25);   // true
print percentage_validator(150);  // false
```

### 3. Callback Patterns

```k
// Event handling with callbacks
mut event_handlers = [];

add_handler = fn(handler) mut(event_handlers) {
    event_handlers.push(handler);
};

trigger_event = fn(data) {
    event_handlers.each(fn(handler) {
        handler(data);
    });
};

// Usage
add_handler(fn(data) {
    print "Handler 1:", data;
});

add_handler(fn(data) {
    print "Handler 2:", data * 2;
});

trigger_event(42);  // Both handlers execute
```

### 4. State Machines with Closures

```k
// State machine implementation
make_state_machine = fn(initial_state) {
    mut current_state = initial_state;
    mut transitions = {};
    
    return {
        add_transition: fn(from, to, condition) mut(transitions) {
            if (!transitions.has(from)) {
                transitions[from] = [];
            }
            transitions[from].push({ to: to, condition: condition });
        },
        
        update: fn(input) mut(current_state) {
            if (transitions.has(current_state)) {
                for (transition in transitions[current_state]) {
                    if (transition.condition(input)) {
                        current_state = transition.to;
                        break;
                    }
                }
            }
        },
        
        get_state: fn() => current_state
    };
};
```

## Memory Management and Lifetimes

### 1. Automatic Capture Lifetime Management

```k
// Closures automatically manage captured variable lifetimes
make_closure = fn() {
    local_value = 42;  // local to this function
    return fn() => local_value;  // captures local_value
    // local_value lifetime extended by closure
};

closure = make_closure();
print closure();  // 42 - works even though make_closure has returned
```

### 2. Reference Counting for Mutable Captures

```k
// Mutable captures use reference counting
mut shared_data = [1, 2, 3];

closure1 = fn() mut(shared_data) {
    shared_data.push(4);
    return shared_data.length();
};

closure2 = fn() mut(shared_data) {
    shared_data.push(5);
    return shared_data.length();
};

print closure1();  // 4
print closure2();  // 5
// shared_data is [1, 2, 3, 4, 5]
```

## Integration with Variable System

### 1. Function Binding Immutability

```k
// Function bindings are immutable by default
add = fn(x, y) => x + y;
// add = fn(x) => x;  // ERROR: cannot reassign immutable function

// Mutable function binding
mut current_handler = fn(x) => x;
current_handler = fn(x) => x * 2;  // OK: handler is mutable
```

### 2. Captured Variable Mutability Rules

```k
x = 10;          // immutable variable
mut y = 20;      // mutable variable

closure = fn() {
    // x captured as immutable (cannot be changed in closure)
    // y captured as immutable by default
    return x + y;
};

mutable_closure = fn() mut(y) {
    // x still captured as immutable
    // y captured as mutable (can be changed in closure)
    y = y + 1;
    return x + y;
};
```

## Error Handling

### 1. Function Call Errors

```k
x = 42;
x(10);  // ERROR: cannot call non-function value
```

**Error Message:**
```
error: cannot call non-function value
  --> main.k:2:1
   |
2  | x(10);
   |   ^^^^ expected function, found double
```

### 2. Argument Count Mismatch

```k
add = fn(x, y) => x + y;
add(1);  // ERROR: wrong number of arguments
```

**Error Message:**
```
error: function expects 2 arguments, found 1
  --> main.k:2:1
   |
1  | add = fn(x, y) => x + y;
   |       ---------------- function defined here
2  | add(1);
   |     ^^^ expected 2 arguments
```

### 3. Invalid Capture Errors

```k
closure = fn() mut(undefined_var) {  // ERROR: undefined variable
    return undefined_var;
};
```

**Error Message:**
```
error: cannot capture undefined variable 'undefined_var'
  --> main.k:1:20
   |
1  | closure = fn() mut(undefined_var) {
   |                    ^^^^^^^^^^^^^ not found in this scope
```

### 4. Immutable Capture Modification

```k
x = 10;
closure = fn() {
    x = 20;  // ERROR: cannot modify immutable capture
    return x;
};
```

**Error Message:**
```
error: cannot assign to immutable capture 'x'
  --> main.k:3:5
   |
3  |     x = 20;
   |     ^^^^^^ cannot modify immutable capture
   |
help: to modify captured variables, use mut(x) in closure declaration
```

## Implementation Strategy

### 1. AST Node Hierarchy

```cpp
// Base function AST
class FunctionLiteralAST : public ASTNode {
private:
    std::vector<Parameter> parameters;
    std::unique_ptr<ASTNode> body;
    std::vector<CapturedVariable> captures;
    bool is_expression_function;
    
public:
    struct Parameter {
        std::string name;
        bool is_mutable;
        Type type;  // for future type system
    };
    
    struct CapturedVariable {
        std::string name;
        bool is_mutable;
        int scope_level;
    };
    
    FunctionLiteralAST(std::vector<Parameter> params,
                      std::unique_ptr<ASTNode> body,
                      bool is_expr_func = false);
    Value* codegen() override;
    void analyzeCaptures(SymbolTable& symbols);
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
    ReturnStmtAST(std::unique_ptr<ASTNode> val = nullptr);
    Value* codegen() override;
};
```

### 2. Closure Implementation Model

#### Option A: Closure Objects (Recommended)
```cpp
// Runtime closure representation
struct Closure {
    Function* function_ptr;      // LLVM function
    void* capture_context;       // Captured variables
    int ref_count;              // Reference counting
    
    // Capture context layout
    struct CaptureContext {
        // Immutable captures (copied values)
        double immutable_values[N];
        
        // Mutable captures (shared references)
        SharedValue* mutable_refs[M];
    };
};

// Shared value for mutable captures
struct SharedValue {
    double value;
    int ref_count;
};
```

#### Option B: Function Environments
```cpp
// Alternative: lexical environments
struct Environment {
    std::map<std::string, Value> bindings;
    std::shared_ptr<Environment> parent;
};

struct ClosureValue {
    Function* function;
    std::shared_ptr<Environment> env;
};
```

### 3. LLVM Code Generation

```cpp
class FunctionCodeGen {
private:
    // Generate closure allocation
    Value* createClosureObject(FunctionLiteralAST* func);
    
    // Generate capture context
    StructType* createCaptureStruct(const std::vector<CapturedVariable>& captures);
    Value* populateCaptureStruct(const std::vector<CapturedVariable>& captures);
    
    // Generate function with closure context parameter
    Function* generateClosureFunction(FunctionLiteralAST* func);
    
    // Generate closure call
    Value* callClosure(Value* closure, const std::vector<Value*>& args);
    
public:
    Value* codegen(FunctionLiteralAST* func) override;
    Value* codegen(FunctionCallAST* call) override;
};
```

### 4. Memory Management

```cpp
class ClosureManager {
private:
    // Reference counting for closures
    void retainClosure(Closure* closure);
    void releaseClosure(Closure* closure);
    
    // Shared value management
    SharedValue* createSharedValue(double initial_value);
    void retainSharedValue(SharedValue* shared_val);
    void releaseSharedValue(SharedValue* shared_val);
    
public:
    // RAII wrapper for closures
    class ClosureHandle {
        Closure* closure;
    public:
        ClosureHandle(Closure* c) : closure(c) { retainClosure(c); }
        ~ClosureHandle() { releaseClosure(closure); }
        Closure* get() { return closure; }
    };
};
```

## Test Cases

### 1. Basic Function Definition and Call

```k
// TEST: basic_function.k
// EXPECTED: 42.000000000000000
add = fn(x, y) => x + y;
result = add(20, 22);
print result;
```

### 2. Immutable Captures

```k
// TEST: immutable_capture.k  
// EXPECTED: 50.000000000000000
multiplier = 10;
scale = fn(x) => x * multiplier;
result = scale(5);
print result;
```

### 3. Mutable Captures

```k
// TEST: mutable_capture.k
// EXPECTED: 1.000000000000000
// EXPECTED: 2.000000000000000  
// EXPECTED: 3.000000000000000
mut counter = 0;
increment = fn() mut(counter) {
    counter = counter + 1;
    return counter;
};

print increment();
print increment();
print increment();
```

### 4. Higher-Order Functions

```k
// TEST: higher_order.k
// EXPECTED: 25.000000000000000
// EXPECTED: 100.000000000000000
apply = fn(f, x) => f(x);
square = fn(x) => x * x;

print apply(square, 5);
print apply(square, 10);
```

### 5. Nested Closures

```k
// TEST: nested_closures.k
// EXPECTED: 8.000000000000000
// EXPECTED: 15.000000000000000
make_adder = fn(n) => fn(x) => x + n;
add_5 = make_adder(5);
add_10 = make_adder(10);

print add_5(3);   // 8
print add_10(5);  // 15
```

### 6. Mutable Parameters

```k
// TEST: mutable_params.k
// EXPECTED: 15.000000000000000
process = fn(mut x, y) {
    x = x * 2;
    return x + y;
};

result = process(5, 5);  // x becomes 10, result = 10 + 5 = 15
print result;
```

### 7. Complex Closure State

```k
// TEST: closure_state.k
// EXPECTED: 1.000000000000000
// EXPECTED: 3.000000000000000
// EXPECTED: 6.000000000000000
// EXPECTED: 10.000000000000000
make_accumulator = fn(init) {
    mut total = init;
    return fn(value) mut(total) {
        total = total + value;
        return total;
    };
};

acc = make_accumulator(0);
print acc(1);  // 1
print acc(2);  // 3  
print acc(3);  // 6
print acc(4);  // 10
```

## Integration Points

### 1. With Variable System

- Function bindings follow immutability rules
- Captured variables respect mut/immutable declarations
- Parameter mutability is explicitly declared

### 2. With Module System (Future)

```k
// Export functions from modules
export add = fn(x, y) => x + y;
export make_counter = fn() {
    mut count = 0;
    return fn() mut(count) => count = count + 1;
};

// Import and use functions
import { add, make_counter } from "math";
```

### 3. With Type System (Future)

```k
// Type annotations for functions
add: (double, double) -> double = fn(x, y) => x + y;
map: <T, U>(Array<T>, (T) -> U) -> Array<U> = fn(arr, f) => {
    // implementation
};
```

This specification provides a robust foundation for implementing first-class functions and closures in ArithLang, with seamless integration with the immutable variable system and preparation for future language features.