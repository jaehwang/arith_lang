# ArithLang BNF Syntax

```bnf
<program>      ::= <statement>*

<statement>    ::= <assignment> ";"
                 | "print" <print_args> ";"
                 | "if" "(" <expression> ")" <block> "else" <block>
                 | "while" "(" <expression> ")" <block>
                 | <return_stmt>

<print_args>    ::= <string_literal> ("," <expression>)*
                 | <expression>

<block>         ::= "{" <statement>* "}"

<return_stmt>  ::= "return" <expression>? ";"

<assignment>   ::= <mut_qualifier>? <identifier> "=" <expression>
<mut_qualifier>::= "mut"

<expression>   ::= <comparison> ( ("+" | "-") <comparison> )*

<comparison>   ::= <term> ( (">" | "<" | ">=" | "<=" | "==" | "!=") <term> )*

<term>         ::= <factor> ( ("*" | "/") <factor> )*

<factor>       ::= "-" <call_expr>
                 | <call_expr>

<call_expr>    ::= <primary> ( "(" <arg_list>? ")" )*

<arg_list>     ::= <expression> ("," <expression>)*

<primary>      ::= <number>
                 | <identifier>
                 | <string_literal>
                 | "(" <expression> ")"
                 | <function_literal>

<function_literal> ::= "fn" "(" <parameter_list>? ")" <capture_clause>? <function_body>

<function_body>    ::= "=>" <expression>
                     | <block>

<parameter_list>   ::= <parameter> ("," <parameter>)*

<parameter>        ::= "mut"? <identifier>

<capture_clause>   ::= "mut" "(" <identifier> ("," <identifier>)* ")"

<string_literal> ::= '"' <string_content> '"'

<string_content> ::= (<string_char> | <escape_sequence>)*

<string_char>   ::= [^"\\]  // Any character except quote and backslash

<escape_sequence> ::= "\\n" | "\\t" | "\\r" | "\\\"" | "\\\\"

<number>       ::= [0-9]+ ("." [0-9]+)?

<identifier>   ::= [a-zA-Z_][a-zA-Z0-9_]*
```

## Language Features

### First-Class Functions

ArithLang supports first-class function values using the `fn` keyword.

#### Expression Functions
Single-expression functions with `=>` body:
```
add = fn(x, y) => x + y;
result = add(3, 4);
print result;         // 7.000000000000000
```

#### Block Functions
Multi-statement functions with explicit `return`:
```
triple = fn(x) {
    result = x * 3;
    return result;
};
y = triple(14);
print y;              // 42.000000000000000
```

#### Mutable Parameters
Parameters declared with `mut` can be reassigned inside the function body:
```
clamp = fn(mut x, lo, hi) => {
    if (x < lo) { x = lo; } else { x = x; }
    return x;
};
```

#### Closures — Immutable Capture
Variables from outer scope are automatically captured by value at definition time:
```
multiplier = 10;
scale = fn(x) => x * multiplier;
result = scale(5);
print result;         // 50.000000000000000
// Later changes to `multiplier` do not affect `scale`
```

#### Closures — Mutable Capture
Use `mut(var)` clause to share a heap-allocated binding with the outer scope:
```
mut counter = 0;
increment = fn() mut(counter) {
    counter = counter + 1;
    return counter;
};
x = increment();
print x;              // 1.000000000000000
y = increment();
print y;              // 2.000000000000000
```

#### Higher-Order Functions
Functions can be passed as arguments and called inside other functions:
```
apply = fn(f, x) => f(x);
square = fn(x) => x * x;
result = apply(square, 5);
print result;         // 25.000000000000000
```

#### Recursive Functions
A function can call itself by referring to the variable it is assigned to:
```
factorial = fn(n) {
    mut result = 0;
    if (n == 0) { result = 1; } else { result = n * factorial(n - 1); }
    return result;
};
print factorial(5);   // 120.000000000000000
```

### Arithmetic Expressions
- Binary operators: `+`, `-`, `*`, `/`
- **Unary minus operator**: `-` (NEW)
- Proper operator precedence and associativity
- Parentheses for grouping expressions

### Unary Expressions (NEW)
The unary minus operator has higher precedence than all binary operators:

```
-5.0          // Negative number literal
-x            // Negation of variable
-(2 + 3)      // Negation of expression 
--5.0         // Double negation (results in positive 5.0)
-2.0 * 3.0    // Unary minus has higher precedence than multiplication
```

### Comparison Operators
- `>`, `<`, `>=`, `<=`, `==`, `!=`
- Return 1.0 for true, 0.0 for false

### Variables
- Immutable-by-default declaration: `x = 42;`
- Mutable declaration with `mut`: `mut i = 0; i = i + 1;`
- Shadowing allowed: `x = x + 5;` (새 바인딩 생성)
- 참고: 상세 의미론 및 에러 메시지는 `specs/variables.md` 참조

### Control Flow
- **If statements**: `if (condition) { ... } else { ... }`
- **While loops**: `while (condition) { ... }`

### Input/Output
- **Print statement**: Enhanced printf-like functionality
  - Simple expression: `print expression;`
  - String literals: `print "Hello, World!";`
  - Formatted output: `print "Value: %.2f", x;`
  - Multi-argument: `print "x=%f, y=%f", a, b;`
- High-precision output (15 decimal places for numeric expressions)
- **String literals**: Support escape sequences (\n, \t, \", \\, \r)

### Comments
- Single-line comments: `// comment text`

## Example Program

```
// Mutable variable example
mut i = 0;
i = i + 1;

// Calculate using unary minus
x = -1.0;           // Negative assignment
y = -x;             // Negation of variable  
print y;            // Outputs: 1.000000000000000

// Complex expressions with unary minus
result = -(2.0 + 3.0) * -2.0;
print result;       // Outputs: 10.000000000000000

// Enhanced print statements with string literals
print "Hello, World!";              // String literal output
print "Pi = %.5f", 3.14159;         // Formatted output
print "x=%f, y=%f", x, y;           // Multi-argument output

// String literals with escape sequences
print "Line 1\nLine 2";             // Newline escape
print "Quote: \"Hello\"";           // Quote escape

// Conditional with unary expressions
if (-1.0 < 0.0) {
    print "Negative number detected";
} else {
    print "Positive number";
}
```
