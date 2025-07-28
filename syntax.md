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

<factor>       ::= <number>
                 | <identifier>
                 | "(" <expression> ")"

<number>       ::= [0-9]+ ("." [0-9]+)?

<identifier>   ::= [a-zA-Z_][a-zA-Z0-9_]*
```
