# ArithLang Variable System Specification

## Overview

ArithLang adopts an **immutable-by-default** variable system inspired by Rust and modern functional languages. Variables are immutable by default, promoting safer code and clearer reasoning about program state. Mutability must be explicitly declared using the `mut` keyword.

## Implementation Status (현재 구현 상태)

- 구현됨 (현재 동작)
    - 기본 불변(immutable) 변수, `mut`로 명시적 가변 선언
    - 섀도잉(shadowing): 새로운 바인딩으로 재정의 가능
    - 최소 타입 시스템: Number, String
        - 가변 변수 재할당 시 타입 일치 강제
        - 섀도잉(새 바인딩) 시 타입 변경 허용
    - 정밀한 소스 위치 포함 진단: file:line:col + 소스 라인 + 캐럿(^) + note/help
    - 미정의 변수 사용 시 정확한 위치로 에러 보고
- 미구현/로드맵
    - 함수/클로저 문법(`fn`), 가변 매개변수 표기
    - 패턴 매칭(`match`)
    - 구조 분해 할당(튜플/객체)
    - 참조/대여(&, &mut)
    - range/이터레이터 API
    - 모듈 시스템(export/import)
    - 튜플/객체 리터럴 예시는 설명용이며 아직 미구현

## Core Principles

1. **Immutable by Default**: Variables cannot be reassigned after initial binding unless explicitly marked as mutable
2. **Explicit Mutability**: Use `mut` keyword to declare mutable variables
3. **Shadowing Support**: New bindings can shadow previous ones in the same scope
4. **Type Safety**: Mutation preserves type safety (when type system is enabled)
5. **Lexical Scoping**: Variable bindings follow lexical scoping rules

## Syntax

### Variable Declaration and Assignment

```bnf
<assignment>    ::= <mut_qualifier>? <identifier> "=" <expression>
<mut_qualifier> ::= "mut"
```

### Examples

```k
// Immutable binding (default)
x = 42;           // x is bound to 42, cannot be changed
name = "Alice";   // name is bound to "Alice"

// Mutable binding (explicit)
mut counter = 0;  // counter can be reassigned
mut balance = 100.0;

// Assignment to mutable variables
counter = counter + 1;  // OK: counter is mutable
balance = balance - 25.0; // OK: balance is mutable

// Illegal reassignment
// x = 43;  // ERROR: Cannot reassign to immutable variable 'x'
```

## Semantics

### 1. Immutable Variables

#### Declaration and Binding
```k
// Immutable variables are bound once
pi = 3.14159;
message = "Hello, World!";
result = calculate(10, 20);

// These would cause compile-time errors:
// pi = 3.14;        // ERROR: Cannot reassign immutable variable
// message = "Hi";   // ERROR: Cannot reassign immutable variable
```

#### Benefits
- **Safety**: Prevents accidental modification
- **Reasoning**: Easier to understand program behavior
- **Optimization**: Compiler can optimize immutable values
- **Concurrency**: Safe sharing between functions/closures

### 2. Mutable Variables

#### Declaration and Usage
```k
// Mutable variables can be reassigned
mut score = 0;
mut player_name = "Anonymous";
mut game_state = "playing";

// Reassignment is allowed
score = score + 100;
player_name = "Player1";
game_state = "finished";
```

#### Restrictions
```k
mut x = 42;
x = 43;        // OK: same type
// x = "hello";   // ERROR: type mismatch
```

### 3. Shadowing vs Mutation

#### Shadowing (New Binding)
```k
// Shadowing creates a new binding with the same name
x = 10;        // First binding: x = 10
x = x * 2;     // Second binding: x = 20 (shadows first x)
x = "hello";   // Third binding: x = "hello" (different type OK)

// Each binding is still immutable
// The second x cannot be changed to 21
```

#### Mutation (Modifying Existing Variable)
```k
// Mutation modifies the existing variable
mut y = 10;    // Mutable binding: y = 10
y = y * 2;     // Mutation: y becomes 20 (same binding)
// y = "hello";   // ERROR: type mismatch in mutation
```

#### Practical Differences

```k
// Shadowing example: different scopes
fn example1() {
    x = 10;           // outer x
    if (condition) {
        x = x + 5;    // new x = 15 (shadows outer x in this block)
        print x;      // prints 15
    }
    print x;          // prints 10 (original x)
}

// Mutation example: same variable across scopes
fn example2() {
    mut x = 10;       // mutable x
    if (condition) {
        x = x + 5;    // mutates same x to 15
        print x;      // prints 15
    }
    print x;          // prints 15 (same mutated x)
}
```

Note: 위 예시의 `fn` 문법은 현재 미구현이며, 개념 설명을 위한 의사 코드입니다. 섀도잉/뮤테이션 개념만 참고하세요.

### 4. Scope and Lifetime

#### Block Scoping
```k
{
    x = 10;           // x exists in this block
    mut y = 20;       // y exists in this block
    
    {
        x = 30;       // shadows outer x in inner block
        y = 40;       // mutates same y from outer block
        mut z = 50;   // z exists only in inner block
    }
    
    print x;          // prints 10 (outer x)
    print y;          // prints 40 (mutated y)
    // print z;       // ERROR: z not in scope
}
```

#### Function Parameters
Note: 이 섹션의 함수 문법과 매개변수 가변 표기는 현재 미구현(로드맵)입니다.
```k
// Parameters are immutable by default
add = fn(a, b) {
    // a = a + 1;     // ERROR: cannot modify immutable parameter
    return a + b;
};

// Mutable parameters
process = fn(mut x, y) {
    x = x * 2;        // OK: x is mutable parameter
    // y = y + 1;     // ERROR: y is immutable parameter
    return x + y;
};
```

## Advanced Features

### 1. Destructuring Assignment
Note: 본 섹션 전체는 로드맵(미구현)입니다.

```k
// Tuple/array destructuring (future feature)
point = (10, 20);
(x, y) = point;           // x = 10, y = 20 (both immutable)
(mut a, b) = point;       // a mutable, b immutable

// Object destructuring (future feature)  
person = { name: "Alice", age: 30 };
{ name, mut age } = person;  // name immutable, age mutable
```

### 2. Pattern Matching Integration
Note: 본 섹션 전체는 로드맵(미구현)입니다.

```k
// Pattern matching with mutability (future feature)
match value {
    Some(x) => {
        // x is immutable in this branch
        print x;
    },
    Some(mut y) => {
        // y is mutable in this branch
        y = y + 1;
        print y;
    },
    None => print "nothing"
}
```

### 3. Reference and Borrowing (Advanced)
Note: 본 섹션 전체는 로드맵(미구현)입니다.

```k
// Future feature: explicit references
mut data = [1, 2, 3];
immutable_ref = &data;      // immutable reference
mutable_ref = &mut data;    // mutable reference

// immutable_ref[0] = 5;    // ERROR: cannot mutate through immutable ref
mutable_ref[0] = 5;         // OK: can mutate through mutable ref
```

## Error Handling

### 1. Reassignment Errors

```k
x = 42;
x = 43;  // ERROR
```

**Error Message:**
```
main.k:2:1: error: cannot reassign to immutable variable 'x'
x = 43;
^
note: first assignment here: main.k:1:1
help: make this binding mutable with 'mut x = ...'
```

### 2. Uninitialized Variable Errors

```k
print x;  // ERROR: x not defined
```

**Error Message:**
```
main.k:1:7: error: cannot find value 'x' in this scope
print x;
    ^
```

### 3. Type Mismatch in Mutation

```k
mut x = 42;
x = "hello";  // ERROR: type mismatch
```

**Error Message:**
```
main.k:2:1: error: mismatched types
x = "hello";
^
note: expected due to first assignment: main.k:1:5
help: expected number, found string
```

## Implementation Guidelines

### 1. Symbol Table Enhancement

```cpp
struct Symbol {
    std::string name;
    Type type;
    bool is_mutable;
    bool is_initialized;
    SourceLocation declaration_site;
    int scope_level;
};

class SymbolTable {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    
public:
    void enterScope();
    void exitScope();
    void declareVariable(const std::string& name, Type type, bool is_mutable);
    void assignVariable(const std::string& name, const SourceLocation& loc);
    bool canReassign(const std::string& name);
    bool canShadow(const std::string& name);
    Symbol* lookupVariable(const std::string& name);
};
```

### 2. AST Node Updates

```cpp
// Assignment AST with mutability tracking
class AssignmentAST : public ASTNode {
private:
    std::string variable_name;
    std::unique_ptr<ASTNode> value;
    bool is_mutable_declaration;
    bool is_reassignment;
    
public:
    AssignmentAST(const std::string& name, std::unique_ptr<ASTNode> val, 
                  bool is_mut, bool is_reassign);
    Value* codegen() override;
    void typecheck() override;
};
```

### 3. Parser Modifications

```cpp
class Parser {
private:
    std::unique_ptr<AssignmentAST> parseAssignment();
    bool consumeMutKeyword();
    
public:
    // Enhanced parsing for mut keyword
    std::unique_ptr<ASTNode> parseStatement() override;
};
```

### 4. Type Checker Integration

```cpp
void TypeChecker::checkAssignment(AssignmentAST* assignment) {
    Symbol* symbol = symbol_table.lookupVariable(assignment->getName());
    
    if (assignment->isReassignment()) {
        if (symbol && !symbol->is_mutable) {
            throw TypeError("Cannot reassign immutable variable: " + 
                          assignment->getName());
        }
    }
    
    // Type compatibility check for mutations
    if (symbol && assignment->isReassignment()) {
        Type expected = symbol->type;
        Type actual = inferType(assignment->getValue());
        if (!isCompatible(expected, actual)) {
            throw TypeError("Type mismatch in assignment");
        }
    }
}
```

## Compatibility with Existing Code

### Migration Strategy

#### Current ArithLang Code
```k
// Current syntax (will continue to work)
x = 42;
y = x + 10;
print y;
```

#### With New System
```k
// Same code works - variables are immutable by default
x = 42;        // immutable x
y = x + 10;    // immutable y (shadowing is OK)
print y;
```

#### Explicit Mutability Where Needed
```k
// Only loops and counters need mut
mut i = 0;
while (i < 10) {
    print i;
    i = i + 1;  // mutation
}
```

### Backward Compatibility

1. **Existing `.k` files** continue to work unchanged
2. **Shadowing behavior** remains the same for immutable variables
3. **New `mut` keyword** is optional and additive
4. **Error messages** guide users toward correct usage

## Test Cases

### 1. Basic Immutability

```k
// TEST: basic_immutable.k
// EXPECTED_COMPILE: success
x = 42;
y = x + 10;
print y;  // Should print: 52.000000000000000
```

### 2. Mutation Success

```k
// TEST: basic_mutable.k  
// EXPECTED_COMPILE: success
mut counter = 0;
counter = counter + 1;
counter = counter * 2;
print counter;  // Should print: 2.000000000000000
```

### 3. Immutability Error

```k
// TEST: immutable_error.k
// EXPECTED_COMPILE: error
// EXPECTED: error: cannot assign twice to immutable variable 'x'
x = 42;
x = 43;
```

### 4. Shadowing Success

```k
// TEST: shadowing.k
// EXPECTED_COMPILE: success
x = 10;
x = x + 5;  // shadowing creates new x = 15
print x;    // Should print: 15.000000000000000
```

### 5. Function Parameters

```k
// TEST: function_params.k
// EXPECTED_COMPILE: success
process = fn(a, mut b) {
    b = b * 2;
    return a + b;
};

result = process(10, 5);  // a=10, b becomes 10, result = 20
print result;  // Should print: 20.000000000000000
```

### 6. Scope and Lifetime

```k
// TEST: scopes.k
// EXPECTED_COMPILE: success
x = 100;
{
    mut y = 10;
    x = 200;  // shadows outer x
    y = 20;   // mutates inner y
    print x;  // Should print: 200.000000000000000
    print y;  // Should print: 20.000000000000000
}
print x;      // Should print: 100.000000000000000 (original x)
```

## Integration with Other Language Features

### 1. Functions and Closures
Note: 함수/클로저 문법은 현재 미구현(로드맵)입니다.

```k
// Function bindings are immutable
add = fn(x, y) => x + y;
// add = fn(x) => x;  // ERROR: cannot reassign function

// Closure captures can be mutable
mut total = 0;
accumulate = fn(value) mut(total) {
    total = total + value;
    return total;
};
```

### 2. Module System
Note: 모듈 시스템은 현재 미구현(로드맵)입니다.

```k
// Export immutable values
export PI = 3.14159;
export CONFIG = { host: "localhost", port: 8080 };

// Export mutable state (discouraged but allowed)
export mut global_counter = 0;
```

### 3. Control Flow

```k
// Loop variables typically need to be mutable
mut i = 0;
while (i < 10) {
    print i;
    i = i + 1;
}

// Immutable loop control (functional style)
range(0, 10).each(fn(i) {
    print i;  // i is immutable in each iteration
});
```

Note: 위 range/each 및 `fn` 예시는 개념 설명용이며 현재 미구현 기능을 포함합니다.

This specification provides a comprehensive foundation for implementing immutable-by-default variables in ArithLang while maintaining compatibility with existing code and preparing for integration with functions, closures, and modules.