#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Base class for all negative parser tests
class NegativeParserTests : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
    
    // Helper method to expect parsing to throw an exception
    void expectParseError(const std::string& code) {
        EXPECT_THROW({
            Lexer lexer(code);
            Parser parser(lexer);
            auto program = parser.parseProgram();
        }, std::runtime_error) << "Code should have failed to parse: " << code;
    }
    
    // Helper method to expect parsing to throw with specific error message
    void expectParseErrorWithMessage(const std::string& code, const std::string& expectedMessage) {
        try {
            Lexer lexer(code);
            Parser parser(lexer);
            auto program = parser.parseProgram();
            FAIL() << "Expected exception was not thrown for code: " << code;
        } catch (const std::runtime_error& e) {
            EXPECT_THAT(e.what(), testing::HasSubstr(expectedMessage)) 
                << "Error message '" << e.what() << "' does not contain '" << expectedMessage << "'";
        }
    }
};

// 1. Statement Syntax Error Tests
// ============================================================================
// ============================================================================

class StatementSyntaxErrorTests : public NegativeParserTests {};

// 1.1 Missing Semicolons Tests
TEST_F(StatementSyntaxErrorTests, MissingSemicolon_Assignment) {
    expectParseErrorWithMessage("x = 1 y = 2", "Expected ';' after expression statement");
}

TEST_F(StatementSyntaxErrorTests, MissingSemicolon_Print) {
    expectParseErrorWithMessage("print 5", "Expected ';' after print statement");
}

TEST_F(StatementSyntaxErrorTests, MissingSemicolon_LastStatement) {
    expectParseErrorWithMessage("x = 1; print x", "Expected ';' after print statement");
}

TEST_F(StatementSyntaxErrorTests, MissingSemicolon_Multiple) {
    expectParseErrorWithMessage("x = 1 y = 2 z = 3", "Expected ';' after expression statement");
}

TEST_F(StatementSyntaxErrorTests, MissingSemicolon_InBlock) {
    expectParseError("if (x > 0) { print x }");
}

// 1.2 Invalid Statement Keywords
TEST_F(StatementSyntaxErrorTests, InvalidKeyword_For) {
    expectParseError("for (i = 0; i < 10; i++)");
}

TEST_F(StatementSyntaxErrorTests, InvalidKeyword_Function) {
    expectParseError("function test() { }");
}

TEST_F(StatementSyntaxErrorTests, InvalidKeyword_Return) {
    expectParseError("return 42;");
}

// ============================================================================
// 2. Assignment Syntax Error Tests  
// ============================================================================

class AssignmentSyntaxErrorTests : public NegativeParserTests {};

// 2.1 Invalid Assignment Targets
TEST_F(AssignmentSyntaxErrorTests, InvalidAssignTarget_Number) {
    expectParseError("123 = 42;");
}

TEST_F(AssignmentSyntaxErrorTests, InvalidAssignTarget_Expression) {
    expectParseError("(x + 1) = 42;");
}

TEST_F(AssignmentSyntaxErrorTests, InvalidAssignTarget_String) {
    expectParseError("\"hello\" = 42;");
}

// 2.2 Missing Assignment Components
TEST_F(AssignmentSyntaxErrorTests, MissingAssignValue) {
    expectParseError("x = ;");
}

TEST_F(AssignmentSyntaxErrorTests, MissingAssignOperator) {
    expectParseErrorWithMessage("x 42;", "Expected ';' after expression statement");
}

TEST_F(AssignmentSyntaxErrorTests, MissingAssignTarget) {
    expectParseError("= 42;");
}

// ============================================================================
// 3. Expression Syntax Error Tests
// ============================================================================

class ExpressionSyntaxErrorTests : public NegativeParserTests {};

// 3.1 Incomplete Binary Operations
TEST_F(ExpressionSyntaxErrorTests, IncompleteAddition) {
    expectParseError("x = 10 + ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteSubtraction) {
    expectParseError("x = 10 - ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteMultiplication) {
    expectParseError("x = 10 * ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteDivision) {
    expectParseError("x = 10 / ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteComparison_GT) {
    expectParseError("x = 10 > ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteComparison_LT) {
    expectParseError("x = 10 < ;");
}

TEST_F(ExpressionSyntaxErrorTests, IncompleteComparison_EQ) {
    expectParseError("x = 10 == ;");
}

// 3.2 Invalid Operator Sequences
TEST_F(ExpressionSyntaxErrorTests, DoubleAddition) {
    expectParseError("x = 10 + + 5;");
}

TEST_F(ExpressionSyntaxErrorTests, MixedOperators) {
    expectParseError("x = 10 +* 5;");
}

TEST_F(ExpressionSyntaxErrorTests, OperatorAtStart) {
    expectParseError("x = + 5;");
}

TEST_F(ExpressionSyntaxErrorTests, OperatorAtStart_Multiply) {
    expectParseError("x = * 5;");
}

// 3.3 Missing Operands
TEST_F(ExpressionSyntaxErrorTests, MissingLeftOperand) {
    expectParseError("x = + 5;");
}

TEST_F(ExpressionSyntaxErrorTests, MissingRightOperand) {
    expectParseError("x = 5 +;");
}

TEST_F(ExpressionSyntaxErrorTests, MissingBothOperands) {
    expectParseError("x = +;");
}

// ============================================================================
// 4. Primary Expression Error Tests
// ============================================================================

class PrimaryExpressionErrorTests : public NegativeParserTests {};

// 4.1 Parentheses Mismatches
TEST_F(PrimaryExpressionErrorTests, UnmatchedOpenParen) {
    expectParseError("x = (10 + 5;");
}

TEST_F(PrimaryExpressionErrorTests, UnmatchedCloseParen) {
    expectParseError("x = 10 + 5);");
}

TEST_F(PrimaryExpressionErrorTests, EmptyParentheses) {
    expectParseError("x = ();");
}

TEST_F(PrimaryExpressionErrorTests, NestedUnmatched_Open) {
    expectParseError("x = ((10 + 5);");
}

TEST_F(PrimaryExpressionErrorTests, NestedUnmatched_Close) {
    expectParseError("x = (10 + 5));");
}

// 4.2 Invalid Number Formats
TEST_F(PrimaryExpressionErrorTests, InvalidNumber_StartingDot) {
    expectParseError("x = .123;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidNumber_EndingDot) {
    expectParseError("x = 123.;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidNumber_DoubleDot) {
    expectParseError("x = 12.34.56;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidNumber_Scientific) {
    expectParseError("x = 1.23e5;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidNumber_Hex) {
    expectParseError("x = 0xFF;");
}

// 4.3 Invalid Identifier Names
TEST_F(PrimaryExpressionErrorTests, InvalidId_StartingDigit) {
    expectParseError("123invalid = 42;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidId_SpecialChar) {
    expectParseError("x-var = 42;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidId_Space) {
    expectParseError("x var = 42;");
}

TEST_F(PrimaryExpressionErrorTests, InvalidId_Dot) {
    expectParseError("x.var = 42;");
}

// ============================================================================
// 5. Control Flow Syntax Error Tests
// ============================================================================

class ControlFlowSyntaxErrorTests : public NegativeParserTests {};

// 5.1 If Statement Errors
TEST_F(ControlFlowSyntaxErrorTests, If_MissingOpenParen) {
    expectParseError("if x > 0) { print 1; } else { print 0; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingCloseParen) {
    expectParseError("if (x > 0 { print 1; } else { print 0; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingCondition) {
    expectParseError("if () { print 1; } else { print 0; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingOpenBrace) {
    expectParseError("if (x > 0) print 1; } else { print 0; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingCloseBrace) {
    expectParseError("if (x > 0) { print 1; else { print 0; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingElse) {
    expectParseError("if (x > 0) { print 1; }");
}

TEST_F(ControlFlowSyntaxErrorTests, If_MissingElseBlock) {
    expectParseError("if (x > 0) { print 1; } else");
}

// 5.2 While Loop Errors
TEST_F(ControlFlowSyntaxErrorTests, While_MissingOpenParen) {
    expectParseError("while x > 0) { print 1; }");
}

TEST_F(ControlFlowSyntaxErrorTests, While_MissingCloseParen) {
    expectParseError("while (x > 0 { print 1; }");
}

TEST_F(ControlFlowSyntaxErrorTests, While_MissingCondition) {
    expectParseError("while () { print 1; }");
}

TEST_F(ControlFlowSyntaxErrorTests, While_MissingOpenBrace) {
    expectParseError("while (x > 0) print 1; }");
}

TEST_F(ControlFlowSyntaxErrorTests, While_MissingCloseBrace) {
    expectParseError("while (x > 0) { print 1;");
}

// ============================================================================
// 6. Block Syntax Error Tests
// ============================================================================

class BlockSyntaxErrorTests : public NegativeParserTests {};

TEST_F(BlockSyntaxErrorTests, Block_MissingOpenBrace) {
    expectParseError("if (x > 0) print 1; } else { print 0; }");
}

TEST_F(BlockSyntaxErrorTests, Block_MissingCloseBrace) {
    expectParseError("if (x > 0) { print 1; else { print 0; }");
}

TEST_F(BlockSyntaxErrorTests, Block_UnmatchedBraces) {
    expectParseError("{ print x; { print y; }");
}

// Test that empty blocks should be valid (this is a positive test case)
TEST_F(BlockSyntaxErrorTests, Block_EmptyBlocks_ShouldBeValid) {
    // This should NOT throw an exception
    EXPECT_NO_THROW({
        Lexer lexer("if (x > 0) { } else { }");
        Parser parser(lexer);
        auto program = parser.parseProgram();
    });
}

// ============================================================================
// 7. Print Statement Error Tests
// ============================================================================

class PrintStatementSyntaxErrorTests : public NegativeParserTests {};

// 7.1 Basic Print Syntax Errors
TEST_F(PrintStatementSyntaxErrorTests, Print_MissingExpression) {
    expectParseError("print;");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_InvalidExpression) {
    expectParseError("print +;");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_MissingSemicolon) {
    expectParseErrorWithMessage("print 42", "Expected ';' after print statement");
}

// 7.2 Print String Literal Errors (these tests assume string literal support)
TEST_F(PrintStatementSyntaxErrorTests, Print_UnterminatedString) {
    expectParseError("print \"Hello;");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_UnterminatedString_EOF) {
    expectParseError("print \"Hello");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_InvalidEscape) {
    expectParseError("print \"Hello\\x\";");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_MissingComma) {
    expectParseError("print \"Value\" x;");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_TrailingComma) {
    expectParseError("print \"Value\", x,;");
}

TEST_F(PrintStatementSyntaxErrorTests, Print_CommaWithoutArgs) {
    expectParseError("print \"Value\",;");
}

// ============================================================================
// 8. String Literal Error Tests
// ============================================================================

class StringLiteralSyntaxErrorTests : public NegativeParserTests {};

// 8.1 String Literal Format Errors
TEST_F(StringLiteralSyntaxErrorTests, String_UnterminatedQuote) {
    expectParseError("x = \"hello;");
}

TEST_F(StringLiteralSyntaxErrorTests, String_UnterminatedQuote_EOF) {
    expectParseError("x = \"hello");
}

TEST_F(StringLiteralSyntaxErrorTests, String_InvalidEscape_Unknown) {
    expectParseError("x = \"hello\\x\";");
}

TEST_F(StringLiteralSyntaxErrorTests, String_InvalidEscape_Incomplete) {
    expectParseError("x = \"hello\\\";");
}

// 8.2 String Literal Context Errors
TEST_F(StringLiteralSyntaxErrorTests, String_InArithmetic) {
    expectParseError("x = \"hello\" + 5;");
}

TEST_F(StringLiteralSyntaxErrorTests, String_InComparison) {
    expectParseError("x = \"hello\" > 5;");
}

TEST_F(StringLiteralSyntaxErrorTests, String_AsUnaryOperand) {
    expectParseError("x = -\"hello\";");
}

// Test that string in parentheses should be valid (positive test case)
TEST_F(StringLiteralSyntaxErrorTests, String_InParentheses_ShouldBeValid) {
    // This test will need to be enabled once string literals are implemented
    // EXPECT_NO_THROW({
    //     Lexer lexer("x = (\"hello\");");
    //     Parser parser(lexer);
    //     auto program = parser.parseProgram();
    // });
}

// ============================================================================
// 9. Unary Expression Error Tests
// ============================================================================

class UnaryExpressionErrorTests : public NegativeParserTests {};

// 9.1 Unary Minus Errors
TEST_F(UnaryExpressionErrorTests, Unary_MissingOperand) {
    expectParseError("x = -;");
}

TEST_F(UnaryExpressionErrorTests, Unary_InvalidOperand) {
    expectParseError("x = - +;");
}

// ============================================================================
// 10. Parameterized Tests for Systematic Coverage
// ============================================================================

struct NegativeTestCase {
    std::string name;
    std::string code;
    std::string expectedErrorSubstring;
};

class ParametrizedNegativeTests : public ::testing::TestWithParam<NegativeTestCase> {};

TEST_P(ParametrizedNegativeTests, InvalidSyntaxThrowsError) {
    auto testCase = GetParam();
    
    try {
        Lexer lexer(testCase.code);
        Parser parser(lexer);
        auto program = parser.parseProgram();
        FAIL() << "Expected exception was not thrown for test case: " << testCase.name 
               << " with code: " << testCase.code;
    } catch (const std::runtime_error& e) {
        if (!testCase.expectedErrorSubstring.empty()) {
            EXPECT_THAT(e.what(), testing::HasSubstr(testCase.expectedErrorSubstring))
                << "Test case: " << testCase.name << " - Error message '" << e.what() 
                << "' does not contain '" << testCase.expectedErrorSubstring << "'";
        }
        // Exception was thrown as expected
    }
}

// Parameterized test instances for missing semicolons
INSTANTIATE_TEST_SUITE_P(
    MissingSemicolonTests,
    ParametrizedNegativeTests,
    ::testing::Values(
        NegativeTestCase{"Assignment", "x = 1 y = 2", "Expected ';'"},
        NegativeTestCase{"Print", "print 5", "Expected ';'"},
        NegativeTestCase{"LastStatement", "x = 1; print x", "Expected ';'"},
        NegativeTestCase{"Multiple", "x = 1 y = 2 z = 3", "Expected ';'"}
    )
);

// Parameterized test instances for incomplete expressions
INSTANTIATE_TEST_SUITE_P(
    IncompleteExpressionTests,
    ParametrizedNegativeTests,
    ::testing::Values(
        NegativeTestCase{"IncompleteAddition", "x = 10 + ;", ""},
        NegativeTestCase{"IncompleteSubtraction", "x = 10 - ;", ""},
        NegativeTestCase{"IncompleteMultiplication", "x = 10 * ;", ""},
        NegativeTestCase{"IncompleteDivision", "x = 10 / ;", ""},
        NegativeTestCase{"IncompleteComparison", "x = 10 > ;", ""}
    )
);

// Parameterized test instances for parentheses errors
INSTANTIATE_TEST_SUITE_P(
    ParenthesesErrorTests,
    ParametrizedNegativeTests,
    ::testing::Values(
        NegativeTestCase{"UnmatchedOpen", "x = (10 + 5;", ""},
        NegativeTestCase{"UnmatchedClose", "x = 10 + 5);", ""},
        NegativeTestCase{"EmptyParens", "x = ();", ""},
        NegativeTestCase{"NestedUnmatchedOpen", "x = ((10 + 5);", ""},
        NegativeTestCase{"NestedUnmatchedClose", "x = (10 + 5));", ""}
    )
);
