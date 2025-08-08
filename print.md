# Print Statement Specification

## Overview
The `print` statement in ArithLang provides output functionality similar to C's `printf()`, allowing formatted output of expressions and string literals.

## Current Implementation

### Basic Print Statement
```
print <expression>;
```

**Syntax:**
- Keyword: `print`
- Expression: Any valid arithmetic expression or variable reference
- Terminator: Semicolon (`;`)

**Example:**
```k
x = 42.5;
print x;        // Output: 42.500000000000000
print x * 2;    // Output: 85.000000000000000
```

**Output Format:**
- Fixed precision: 15 decimal places (`%.15f` format)
- Automatic newline appended
- Double precision floating point numbers

### Current Limitations
1. **No string literal support**: `print "Hello";` fails with "Unknown character: '"
2. **Single expression only**: Cannot combine text and values in one statement
3. **Fixed format**: No format specifier control

## Proposed C printf-like Enhancement

### Extended Syntax
```
print <format_string> [, <expression>, ...];
print <expression>;  // Maintains backward compatibility
```

### String Literal Support

#### Lexer Changes
- Add `TOK_STRING` token type
- Support quoted strings: `"text"`
- Handle escape sequences: `\n`, `\t`, `\"`, `\\`

#### Parser Changes
- Extend print statement grammar:
  ```
  print_stmt → 'print' (string_literal | expression) (',' expression)* ';'
  ```
- Add `StringLiteralAST` node type

#### String Literals in AST
```cpp
class StringLiteralAST : public ExprAST {
    std::string value;
public:
    StringLiteralAST(const std::string& value) : value(value) {}
    llvm::Value* codegen() override;
    const std::string& getValue() const { return value; }
};
```

### Format String Implementation

#### Basic Format Specifiers
| Specifier | Type | Description |
|-----------|------|-------------|
| `%f` | double | Floating point (default precision) |
| `%.Nf` | double | Floating point with N decimal places |
| `%g` | double | General format (scientific/fixed) |
| `%e` | double | Scientific notation |
| `%d` | int | Integer (cast from double) |
| `%s` | string | String literal |
| `%%` | literal | Literal percent sign |

#### Examples
```k
// String output
print "Hello, World!";
// Output: Hello, World!

// Formatted numeric output  
x = 3.14159265358979;
print "Pi = %.2f", x;
// Output: Pi = 3.14

print "Scientific: %e", x;
// Output: Scientific: 3.141593e+00

// Mixed output
print "Value: %f, Double: %f", x, x * 2;
// Output: Value: 3.141592653589790, Double: 6.283185307179580

// Backward compatibility
print x;
// Output: 3.141592653589790
```

### Implementation Strategy

#### Phase 1: String Literals
1. **Lexer Extension**
   - Add string token recognition
   - Handle quoted strings and escape sequences
   - Position tracking for error reporting

2. **Parser Extension**
   - Add string literal parsing
   - Update print statement parsing
   - Create `StringLiteralAST` nodes

3. **Code Generation**
   - LLVM string constant creation
   - String-only print statements

#### Phase 2: Format Strings
1. **Format Parser**
   - Parse format specifiers in strings
   - Validate format/argument matching
   - Generate appropriate printf calls

2. **Multi-argument Support**
   - Extend print AST for multiple expressions
   - Variable argument printf calls in LLVM
   - Type checking for format compatibility

#### Phase 3: Advanced Features
1. **Escape Sequences**
   - `\n` (newline), `\t` (tab), `\"` (quote)
   - `\\` (backslash), `\r` (carriage return)

2. **Width and Precision**
   - `%10.2f` (width 10, 2 decimal places)
   - `%-10.2f` (left-justified)
   - `%010.2f` (zero-padded)

### Code Generation Details

#### String Constants
```cpp
llvm::Value* StringLiteralAST::codegen() {
    return codeGenInstance->getBuilder().CreateGlobalString(value, "str");
}
```

#### Print Statement Processing
The `PrintStmtAST::codegen()` method handles three different cases:

1. **Format string with arguments** (e.g., `print "Value: %f", x;`)
2. **String literal only** (e.g., `print "Hello";`)
3. **Numeric expression only** (e.g., `print x;`)

```cpp
llvm::Value* PrintStmtAST::codegen() {
    llvm::Function* printfFunc = codeGenInstance->getPrintfDeclaration();
    llvm::Value* formatVal = formatExpr->codegen();
    
    StringLiteralAST* formatString = dynamic_cast<StringLiteralAST*>(formatExpr.get());
    
    if (formatString && !args.empty()) {
        // Format string with arguments - parse format specifiers
        return generateFormattedPrint(formatString, args);
    } else if (formatString) {
        // String literal only - use %s format without automatic newline
        llvm::Constant* formatStr = CreateGlobalString("%s");
        return CreateCall(printfFunc, {formatStr, formatVal});
    } else {
        // Numeric expression - use %.15f format with automatic newline
        llvm::Constant* formatStr = CreateGlobalString("%.15f\n");
        return CreateCall(printfFunc, {formatStr, formatVal});
    }
}
```

#### Format String Parser
The format string parser supports:
- **Simple specifiers**: `%f`, `%g`, `%e`, `%d`, `%s`
- **Precision specifiers**: `%.Nf` (e.g., `%.3f`)
- **Literal percent**: `%%`
- **Argument validation**: Ensures argument count matches format specifiers

#### Newline Behavior
**Important**: Automatic newline insertion follows these rules:
- **Format strings with arguments**: No automatic newline (user controls via format string)
- **String literals only**: No automatic newline (user controls via string content)
- **Numeric expressions only**: Automatic newline added (backward compatibility)

Examples:
```cpp
print "Hello";          // Output: "Hello" (no newline)
print "Hello\n";        // Output: "Hello" (with newline)
print "Value: %f", x;   // Output: "Value: 3.14" (no newline)
print x;                // Output: "3.14..." (with newline)
```

### Backward Compatibility
- Current `print <expression>;` syntax remains unchanged
- Existing `.k` files continue to work
- Default precision maintains 15 decimal places for expressions
- No breaking changes to existing functionality

### Error Handling
- **Lexer errors**: Unterminated strings, invalid escape sequences
- **Parser errors**: Mismatched format specifiers and arguments
- **Type errors**: Format specifier incompatible with argument type
- **Runtime errors**: Invalid format strings (detected at compile time when possible)

### Test Cases
```k
// String literals (no automatic newline)
print "Simple string";              // Output: Simple string
print "String with\nnewline";       // Output: String with
                                     //         newline
print "Quote: \"Hello\"";           // Output: Quote: "Hello"

// Format strings with arguments (no automatic newline)
print "Integer: %d", 42.7;          // Output: Integer: 42
print "Float: %.3f", 3.14159;       // Output: Float: 3.142
print "Scientific: %e", 1234.5;     // Output: Scientific: 1.234500e+03
print "Value: %f\n", 3.14;          // Output: Value: 3.140000
                                     //         (with newline)

// Mixed format
print "x=%f, y=%f, sum=%f", 1.5, 2.5, 4.0;  
// Output: x=1.500000, y=2.500000, sum=4.000000

// String format specifier
print "Name: %s", "ArithLang";       // Output: Name: ArithLang

// Literal percent
print "Progress: %% complete";       // Output: Progress: % complete

// Backward compatibility (automatic newline preserved)
print 42.5;                          // Output: 42.500000000000000
                                     //         (with newline)
```

### Future Extensions
1. **Variable argument count validation**
2. **Additional format specifiers** (`%x` for hex, `%o` for octal)
3. **String variable support** (requires string data type)
4. **Printf return value** (number of characters printed)
5. **Sprintf-like functionality** (format to string variable)

This specification maintains the educational and simple nature of ArithLang while adding practical output formatting capabilities similar to C's printf family of functions.
