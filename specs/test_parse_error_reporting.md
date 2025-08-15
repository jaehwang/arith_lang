# Parse Error Reporting Test Specification

## Overview
This document defines comprehensive test cases for the Parse Error Reporting feature. These tests are designed for Test-Driven Development (TDD) and cover both positive and negative scenarios to ensure robust error location tracking and reporting.

## Test Categories

### Phase 1: Core Location Tracking Tests
### Phase 2: Enhanced Error Display Tests  
### Phase 3: Edge Cases and Integration Tests

---

## Phase 1: Core Location Tracking Tests

### 1.1 SourceLocation Basic Tests

#### Test: `SourceLocation_DefaultConstruction`
```cpp
TEST(SourceLocationTest, DefaultConstruction) {
    SourceLocation loc;
    EXPECT_EQ(loc.file, "");
    EXPECT_EQ(loc.line, 1);
    EXPECT_EQ(loc.column, 1);
}
```

#### Test: `SourceLocation_CustomConstruction`
```cpp
TEST(SourceLocationTest, CustomConstruction) {
    SourceLocation loc{"test.k", 5, 10};
    EXPECT_EQ(loc.file, "test.k");
    EXPECT_EQ(loc.line, 5);
    EXPECT_EQ(loc.column, 10);
}
```

### 1.2 SourceRange Tests

#### Test: `SourceRange_SingleCharacterToken`
```cpp
TEST(SourceRangeTest, SingleCharacterToken) {
    SourceLocation start{"test.k", 1, 5};
    SourceLocation end{"test.k", 1, 6};
    SourceRange range{start, end};
    
    EXPECT_EQ(range.start.column, 5);
    EXPECT_EQ(range.end.column, 6);
    EXPECT_EQ(range.end.column - range.start.column, 1);
}
```

#### Test: `SourceRange_MultiCharacterToken`
```cpp
TEST(SourceRangeTest, MultiCharacterToken) {
    SourceLocation start{"test.k", 1, 10};
    SourceLocation end{"test.k", 1, 15};  // 5-character token
    SourceRange range{start, end};
    
    EXPECT_EQ(range.end.column - range.start.column, 5);
}
```

### 1.3 Token Range Tests

#### Test: `Token_WithSourceRange`
```cpp
TEST(TokenTest, WithSourceRange) {
    SourceRange range{{"test.k", 1, 5}, {"test.k", 1, 7}};
    Token token(TOK_NUMBER, "42", 42.0, range);
    
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 5);
    EXPECT_EQ(token.range.end.column, 7);
}
```

#### Test: `Token_DefaultRange`
```cpp
TEST(TokenTest, DefaultRange) {
    Token token(TOK_PLUS, "+");
    // Default range should be empty/default constructed
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 1);
}
```

### 1.4 Lexer Location Tracking Tests

#### Test: `Lexer_BasicLocationTracking`
**Input**: `"42"`
**Expected**:
```cpp
TEST(LexerLocationTest, BasicLocationTracking) {
    Lexer lexer("42", "test.k");
    Token token = lexer.getNextToken();
    
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_EQ(token.range.start.file, "test.k");
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 1);
    EXPECT_EQ(token.range.end.column, 3);
}
```

#### Test: `Lexer_MultipleTokensOnSameLine`
**Input**: `"x + 42"`
**Expected**:
```cpp
TEST(LexerLocationTest, MultipleTokensOnSameLine) {
    Lexer lexer("x + 42", "test.k");
    
    // Token 1: 'x' at column 1
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.range.start.column, 1);
    EXPECT_EQ(token1.range.end.column, 2);
    
    // Token 2: '+' at column 3
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_PLUS);
    EXPECT_EQ(token2.range.start.column, 3);
    EXPECT_EQ(token2.range.end.column, 4);
    
    // Token 3: '42' at column 5
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.range.start.column, 5);
    EXPECT_EQ(token3.range.end.column, 7);
}
```

#### Test: `Lexer_NewlineHandling`
**Input**: `"x\ny"`
**Expected**:
```cpp
TEST(LexerLocationTest, NewlineHandling) {
    Lexer lexer("x\ny", "test.k");
    
    // Token 1: 'x' at line 1, column 1
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.range.start.line, 1);
    EXPECT_EQ(token1.range.start.column, 1);
    
    // Token 2: 'y' at line 2, column 1
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.range.start.line, 2);
    EXPECT_EQ(token2.range.start.column, 1);
}
```

#### Test: `Lexer_TabHandling`
**Input**: `"x\ty"`
**Expected**:
```cpp
TEST(LexerLocationTest, TabHandling) {
    Lexer lexer("x\ty", "test.k");
    
    // Token 1: 'x' at column 1
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.range.start.column, 1);
    
    // Token 2: 'y' at column 3 (tab counts as 1 column)
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.range.start.column, 3);
}
```

#### Test: `Lexer_MultiCharacterOperators`
**Input**: `"x >= y"`
**Expected**:
```cpp
TEST(LexerLocationTest, MultiCharacterOperators) {
    Lexer lexer("x >= y", "test.k");
    
    lexer.getNextToken(); // skip 'x'
    
    Token token = lexer.getNextToken(); // '>='
    EXPECT_EQ(token.type, TOK_GTE);
    EXPECT_EQ(token.range.start.column, 3);
    EXPECT_EQ(token.range.end.column, 5); // 2-character operator
}
```

#### Test: `Lexer_StringLiteralWithEscapes`
**Input**: `"\"hello\\nworld\""`
**Expected**:
```cpp
TEST(LexerLocationTest, StringLiteralWithEscapes) {
    Lexer lexer("\"hello\\nworld\"", "test.k");
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_STRING);
    EXPECT_EQ(token.range.start.column, 1);
    EXPECT_EQ(token.range.end.column, 15); // Full string length
}
```

#### Test: `Lexer_EOFToken`
**Input**: `"x"`
**Expected**:
```cpp
TEST(LexerLocationTest, EOFToken) {
    Lexer lexer("x", "test.k");
    
    lexer.getNextToken(); // consume 'x'
    Token eofToken = lexer.getNextToken();
    
    EXPECT_EQ(eofToken.type, TOK_EOF);
    EXPECT_EQ(eofToken.range.start.column, 2); // after 'x'
    EXPECT_EQ(eofToken.range.start.column, eofToken.range.end.column); // EOF is zero-width
}
```

### 1.5 ParseError Exception Tests

#### Test: `ParseError_BasicConstruction`
```cpp
TEST(ParseErrorTest, BasicConstruction) {
    SourceLocation loc{"test.k", 5, 10};
    ParseError error("Test message", loc);
    
    EXPECT_STREQ(error.what(), "Test message");
    EXPECT_EQ(error.loc.file, "test.k");
    EXPECT_EQ(error.loc.line, 5);
    EXPECT_EQ(error.loc.column, 10);
}
```

#### Test: `ParseError_ThrowAndCatch`
```cpp
TEST(ParseErrorTest, ThrowAndCatch) {
    SourceLocation loc{"test.k", 3, 7};
    
    try {
        throw ParseError("Syntax error", loc);
        FAIL() << "Expected ParseError to be thrown";
    } catch (const ParseError& e) {
        EXPECT_STREQ(e.what(), "Syntax error");
        EXPECT_EQ(e.loc.line, 3);
        EXPECT_EQ(e.loc.column, 7);
    }
}
```

### 1.6 Parser Error Helper Tests

#### Test: `Parser_ErrorHereHelper`
```cpp
TEST(ParserErrorTest, ErrorHereHelper) {
    std::string source = "x + ";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    // Position parser at EOF token
    parser.getNextToken(); // x
    parser.getNextToken(); // +
    parser.getNextToken(); // EOF
    
    try {
        parser.errorHere("Unexpected end of input");
        FAIL() << "Expected ParseError to be thrown";
    } catch (const ParseError& e) {
        EXPECT_STREQ(e.what(), "Unexpected end of input");
        EXPECT_EQ(e.loc.column, 4); // EOF position
    }
}
```

#### Test: `Parser_ErrorAtHelper`
```cpp
TEST(ParserErrorTest, ErrorAtHelper) {
    std::string source = "x = (y + z";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    SourceLocation parenLoc{"test.k", 1, 5}; // position of '('
    
    try {
        parser.errorAt("Expected ')'", parenLoc);
        FAIL() << "Expected ParseError to be thrown";
    } catch (const ParseError& e) {
        EXPECT_STREQ(e.what(), "Expected ')'");
        EXPECT_EQ(e.loc.column, 5);
    }
}
```

---

## Phase 2: Enhanced Error Display Tests

### 2.1 Error Message Formatting Tests

#### Test: `ErrorDisplay_BasicFormat`
**Input**: Parse error at line 1, column 5
**Expected Output**:
```
test.k:1:5: error: Expected ';' after print statement
```

```cpp
TEST(ErrorDisplayTest, BasicFormat) {
    SourceLocation loc{"test.k", 1, 5};
    ParseError error("Expected ';' after print statement", loc);
    
    testing::internal::CaptureStderr();
    printParseError(error, "print 42");
    std::string output = testing::internal::GetCapturedStderr();
    
    EXPECT_THAT(output, HasSubstr("test.k:1:5: error: Expected ';' after print statement"));
}
```

#### Test: `ErrorDisplay_WithSourceSnippet`
**Input**: `"print 42"` with error at column 8
**Expected Output**:
```
test.k:1:8: error: Expected ';' after print statement
print 42
       ^
```

```cpp
TEST(ErrorDisplayTest, WithSourceSnippet) {
    SourceLocation loc{"test.k", 1, 8};
    ParseError error("Expected ';' after print statement", loc);
    
    testing::internal::CaptureStderr();
    printParseError(error, "print 42");
    std::string output = testing::internal::GetCapturedStderr();
    
    EXPECT_THAT(output, HasSubstr("print 42"));
    EXPECT_THAT(output, HasSubstr("       ^")); // 7 spaces + caret
}
```

#### Test: `ErrorDisplay_MultipleLines`
**Input**: Multi-line source with error on line 2
**Expected Output**:
```
test.k:2:3: error: Invalid assignment target  
123 = 42;
  ^
```

```cpp
TEST(ErrorDisplayTest, MultipleLines) {
    std::string source = "x = 1;\n123 = 42;\ny = 3;";
    SourceLocation loc{"test.k", 2, 3};
    ParseError error("Invalid assignment target", loc);
    
    testing::internal::CaptureStderr();
    printParseError(error, source);
    std::string output = testing::internal::GetCapturedStderr();
    
    EXPECT_THAT(output, HasSubstr("test.k:2:3: error: Invalid assignment target"));
    EXPECT_THAT(output, HasSubstr("123 = 42;"));
    EXPECT_THAT(output, HasSubstr("  ^")); // 2 spaces + caret
}
```

### 2.2 getLine Helper Function Tests

#### Test: `GetLine_FirstLine`
```cpp
TEST(GetLineTest, FirstLine) {
    std::string source = "line1\nline2\nline3";
    std::string result = getLine(source, 1);
    EXPECT_EQ(result, "line1");
}
```

#### Test: `GetLine_MiddleLine`
```cpp
TEST(GetLineTest, MiddleLine) {
    std::string source = "line1\nline2\nline3";
    std::string result = getLine(source, 2);
    EXPECT_EQ(result, "line2");
}
```

#### Test: `GetLine_LastLine`
```cpp
TEST(GetLineTest, LastLine) {
    std::string source = "line1\nline2\nline3";
    std::string result = getLine(source, 3);
    EXPECT_EQ(result, "line3");
}
```

#### Test: `GetLine_LastLineNoNewline`
```cpp
TEST(GetLineTest, LastLineNoNewline) {
    std::string source = "line1\nline2\nline3";
    std::string result = getLine(source, 3);
    EXPECT_EQ(result, "line3");
}
```

#### Test: `GetLine_InvalidLineNumber`
```cpp
TEST(GetLineTest, InvalidLineNumber) {
    std::string source = "line1\nline2";
    
    EXPECT_EQ(getLine(source, 0), "");   // Invalid: line 0
    EXPECT_EQ(getLine(source, -1), "");  // Invalid: negative
    EXPECT_EQ(getLine(source, 5), "");   // Invalid: beyond end
}
```

#### Test: `GetLine_EmptySource`
```cpp
TEST(GetLineTest, EmptySource) {
    std::string source = "";
    EXPECT_EQ(getLine(source, 1), "");
}
```

#### Test: `GetLine_OnlyNewlines`
```cpp
TEST(GetLineTest, OnlyNewlines) {
    std::string source = "\n\n\n";
    EXPECT_EQ(getLine(source, 1), "");
    EXPECT_EQ(getLine(source, 2), "");
    EXPECT_EQ(getLine(source, 3), "");
}
```

---

## Phase 3: Integration and Edge Case Tests

### 3.1 Parser Integration Tests

#### Test: `Parser_MissingSemicolonError`
**Input**: `"print 42"`
**Expected**: Error at column 8 pointing after `42`

```cpp
TEST(ParserIntegrationTest, MissingSemicolonError) {
    std::string source = "print 42";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for missing semicolon";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Expected ';'"));
        EXPECT_EQ(e.loc.line, 1);
        EXPECT_EQ(e.loc.column, 8); // After "print 42"
    }
}
```

#### Test: `Parser_UnmatchedParenthesis`
**Input**: `"x = (10 + 5;"`
**Expected**: Error pointing at semicolon

```cpp
TEST(ParserIntegrationTest, UnmatchedParenthesis) {
    std::string source = "x = (10 + 5;";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for unmatched parenthesis";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Expected ')'"));
        EXPECT_EQ(e.loc.column, 12); // At semicolon position
    }
}
```

#### Test: `Parser_InvalidAssignmentTarget`
**Input**: `"123 = 42;"`
**Expected**: Error at column 1 pointing at `123`

```cpp
TEST(ParserIntegrationTest, InvalidAssignmentTarget) {
    std::string source = "123 = 42;";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for invalid assignment target";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Invalid assignment target"));
        EXPECT_EQ(e.loc.column, 1); // At "123"
    }
}
```

#### Test: `Parser_UnterminatedString`
**Input**: `"print \"hello"`
**Expected**: Error at column 7 pointing at opening quote

```cpp
TEST(ParserIntegrationTest, UnterminatedString) {
    std::string source = "print \"hello";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for unterminated string";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Unterminated string"));
        EXPECT_EQ(e.loc.column, 7); // At opening quote
    }
}
```

### 3.2 Main Function Integration Tests

#### Test: `Main_CatchParseError`
```cpp
TEST(MainIntegrationTest, CatchParseError) {
    // This test would require refactoring main() to be testable
    // Mock or create a testable version of the main compilation pipeline
    
    std::string source = "print 42"; // Missing semicolon
    std::string filename = "test.k";
    
    testing::internal::CaptureStderr();
    
    // Call compilation function that should catch ParseError
    int result = compileSource(source, filename);
    
    std::string output = testing::internal::GetCapturedStderr();
    
    EXPECT_EQ(result, 1); // Compilation failure
    EXPECT_THAT(output, HasSubstr("test.k:1:8: error:"));
    EXPECT_THAT(output, HasSubstr("Expected ';'"));
}
```

### 3.3 Edge Cases Tests

#### Test: `EdgeCase_EmptyFile`
**Input**: `""`
**Expected**: EOF token at line 1, column 1

```cpp
TEST(EdgeCaseTest, EmptyFile) {
    Lexer lexer("", "test.k");
    Token token = lexer.getNextToken();
    
    EXPECT_EQ(token.type, TOK_EOF);
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 1);
}
```

#### Test: `EdgeCase_OnlyWhitespace`
**Input**: `"   \t\n  "`
**Expected**: EOF token at appropriate position

```cpp
TEST(EdgeCaseTest, OnlyWhitespace) {
    Lexer lexer("   \t\n  ", "test.k");
    Token token = lexer.getNextToken();
    
    EXPECT_EQ(token.type, TOK_EOF);
    EXPECT_EQ(token.range.start.line, 2);
    EXPECT_EQ(token.range.start.column, 3); // After "  " on line 2
}
```

#### Test: `EdgeCase_VeryLongLine`
**Input**: Line with >120 characters
**Expected**: Proper handling without truncation in Phase 1

```cpp
TEST(EdgeCaseTest, VeryLongLine) {
    std::string longLine(150, 'x'); // 150 x's
    longLine += " = 42;";
    
    SourceLocation loc{"test.k", 1, 152}; // Error near end
    ParseError error("Test error", loc);
    
    testing::internal::CaptureStderr();
    printParseError(error, longLine);
    std::string output = testing::internal::GetCapturedStderr();
    
    // Should display full line in Phase 1
    EXPECT_THAT(output, HasSubstr(longLine));
}
```

#### Test: `EdgeCase_UnicodeCharacters`
**Input**: `"变量 = 42;"`
**Expected**: Proper byte-based column counting

```cpp
TEST(EdgeCaseTest, UnicodeCharacters) {
    // UTF-8 encoded Chinese characters
    std::string source = "变量 = 42;"; // "变量" is 2 chars, 6 bytes
    Lexer lexer(source, "test.k");
    
    Token token1 = lexer.getNextToken(); // identifier
    EXPECT_EQ(token1.range.start.column, 1);
    
    Token token2 = lexer.getNextToken(); // =
    // In byte-based counting: "变量" (6 bytes) + " " (1 byte) = column 8
    EXPECT_EQ(token2.range.start.column, 8);
}
```

#### Test: `EdgeCase_CarriageReturn`
**Input**: `"x\r\ny"`
**Expected**: Proper handling of Windows line endings

```cpp
TEST(EdgeCaseTest, CarriageReturn) {
    Lexer lexer("x\r\ny", "test.k");
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.range.start.line, 1);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.range.start.line, 2);
    EXPECT_EQ(token2.range.start.column, 1);
}
```

### 3.4 Backward Compatibility Tests

#### Test: `BackwardCompatibility_ExistingErrorMessages`
```cpp
TEST(BackwardCompatibilityTest, ExistingErrorMessages) {
    // Test that core error messages remain unchanged
    std::string source = "print 42";
    
    try {
        Lexer lexer(source, "test.k");
        Parser parser(lexer);
        parser.parseProgram();
        FAIL() << "Expected parse error";
    } catch (const ParseError& e) {
        // Core message should remain unchanged
        EXPECT_THAT(e.what(), HasSubstr("Expected ';' after print statement"));
        // Should NOT contain file:line:column (that's added in display)
        EXPECT_THAT(e.what(), Not(HasSubstr("test.k:1:8")));
    }
}
```

#### Test: `BackwardCompatibility_SubstringMatching`
```cpp
TEST(BackwardCompatibilityTest, SubstringMatching) {
    // Existing tests that use substring matching should still work
    SourceLocation loc{"test.k", 1, 5};
    ParseError error("Expected ';' after print statement", loc);
    
    testing::internal::CaptureStderr();
    printParseError(error, "print 42");
    std::string output = testing::internal::GetCapturedStderr();
    
    // Substring matching should still work
    EXPECT_THAT(output, HasSubstr("Expected ';'"));
    EXPECT_THAT(output, HasSubstr("after print statement"));
}
```

---

## Test Organization

### Test Files Structure
```
tests/
├── test_parse_error_reporting.cpp     # Main test file
├── test_source_location.cpp          # SourceLocation/SourceRange tests
├── test_lexer_location_tracking.cpp  # Lexer location tests
├── test_parser_error_helpers.cpp     # Parser error helper tests
├── test_error_display.cpp            # Error display and formatting tests
└── test_integration_error_reporting.cpp # End-to-end integration tests
```

### Test Execution Order
1. **Unit Tests**: Individual component testing
2. **Integration Tests**: Component interaction testing  
3. **Edge Case Tests**: Boundary condition testing
4. **Backward Compatibility Tests**: Regression testing

### Test Data Files
```
tests/data/
├── parse_error_samples/
│   ├── missing_semicolon.k
│   ├── unmatched_paren.k
│   ├── invalid_assignment.k
│   ├── unterminated_string.k
│   └── unicode_test.k
└── expected_outputs/
    ├── missing_semicolon.expected
    ├── unmatched_paren.expected
    └── ...
```

## Test Execution Commands

### Individual Phase Testing
```bash
# Phase 1: Core location tracking
./test_parse_error_reporting --gtest_filter="*LocationTest*:*TokenTest*:*ParseErrorTest*"

# Phase 2: Error display
./test_parse_error_reporting --gtest_filter="*ErrorDisplayTest*:*GetLineTest*"

# Phase 3: Integration and edge cases
./test_parse_error_reporting --gtest_filter="*IntegrationTest*:*EdgeCaseTest*"
```

### Coverage Requirements
- **Line Coverage**: >95% for all modified files
- **Branch Coverage**: >90% for error handling paths
- **Error Path Coverage**: 100% for all defined error conditions

This comprehensive test specification ensures that the Parse Error Reporting feature can be developed using TDD methodology with clear success criteria for each development phase.
