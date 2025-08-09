# ArithLang BNF Syntax

```bnf
<program>      ::= <statement>*

<statement>    ::= <assignment> ";"
                 | "print" <print_args> ";"
                 | "if" "(" <expression> ")" <block> "else" <block>
                 | "while" "(" <expression> ")" <block>

<print_args>    ::= <string_literal> ("," <expression>)*
                 | <expression>

<block>         ::= "{" <statement>* "}"

<assignment>   ::= <identifier> "=" <expression>

<expression>   ::= <comparison> ( ("+" | "-") <comparison> )*

<comparison>   ::= <term> ( (">" | "<" | ">=" | "<=" | "==" | "!=") <term> )*

<term>         ::= <factor> ( ("*" | "/") <factor> )*

<factor>       ::= <unary>
                 | <primary>

<unary>        ::= "-" <primary>

<primary>      ::= <number>
                 | <identifier>
                 | <string_literal>
                 | "(" <expression> ")"

<string_literal> ::= '"' <string_content> '"'

<string_content> ::= (<string_char> | <escape_sequence>)*

<string_char>   ::= [^"\\]  // Any character except quote and backslash

<escape_sequence> ::= "\\n" | "\\t" | "\\r" | "\\\"" | "\\\\"

<number>       ::= [0-9]+ ("." [0-9]+)?

<identifier>   ::= [a-zA-Z_][a-zA-Z0-9_]*
```

## Language Features

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
- Assignment: `variable = expression;`
- Reference: use variable name in expressions

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
