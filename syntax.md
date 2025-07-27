# ArithLang BNF Syntax

```bnf
<program>      ::= <statement>*

<statement>    ::= <assignment> ";"
                 | "print" <expression> ";"

<assignment>   ::= <identifier> "=" <expression>

<expression>   ::= <term> ( ("+" | "-") <term> )*

<term>         ::= <factor> ( ("*" | "/") <factor> )*

<factor>       ::= <number>
                 | <identifier>
                 | "(" <expression> ")"

<number>       ::= [0-9]+ ("." [0-9]+)?

<identifier>   ::= [a-zA-Z_][a-zA-Z0-9_]*
```