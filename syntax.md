# ArithLang BNF Syntax

```bnf
<program>      ::= <statement>*

<statement>    ::= <assignment> ";"
                 | "print" <expression> ";"
                 | "if" "(" <expression> ")" <block> "else" <block>
                 | "while" "(" <expression> ")" <block>

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
                 | "(" <expression> ")"

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
- **Print statement**: `print expression;`
- High-precision output (15 decimal places)

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

// Conditional with unary expressions
if (-1.0 < 0.0) {
    print 1.0;      // This will execute
} else {
    print 0.0;
}
```
