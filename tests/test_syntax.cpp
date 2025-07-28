#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

class SyntaxTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SyntaxTest, AssignmentAndPrint) {
    std::string input = "x=1;print x;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Test first statement: x=1
    auto stmt1 = parser.parseStatement();
    ASSERT_NE(stmt1, nullptr);
    
    // Consume semicolon after assignment
    // The parser should handle this internally, but let's check if we need to advance
    
    // Test second statement: print x;
    auto stmt2 = parser.parseStatement();
    ASSERT_NE(stmt2, nullptr);
}

TEST_F(SyntaxTest, BasicExpression) {
    std::string input = "2 + 3 * 4";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    ASSERT_NE(expr, nullptr);
}

TEST_F(SyntaxTest, ParenthesesExpression) {
    std::string input = "(2 + 3) * 4";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    ASSERT_NE(expr, nullptr);
}

TEST_F(SyntaxTest, VariableAssignmentAndUsage) {
    std::string input = "y = 42; print y;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt1 = parser.parseStatement();
    auto stmt2 = parser.parseStatement();
    ASSERT_NE(stmt1, nullptr);
    ASSERT_NE(stmt2, nullptr);
}

TEST_F(SyntaxTest, ComplexExpression) {
    std::string input = "result = (10 + 5) * 2 - 3; print result;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt1 = parser.parseStatement();
    auto stmt2 = parser.parseStatement();
    ASSERT_NE(stmt1, nullptr);
    ASSERT_NE(stmt2, nullptr);
}

// Comment Processing Tests
class CommentTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CommentTest, SingleLineCommentOnly) {
    std::string input = "// This is a comment";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_EOF);
}

TEST_F(CommentTest, CommentAtEndOfLine) {
    std::string input = "x = 42; // This is a comment";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 42.0);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_SEMICOLON);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_EOF);
}

TEST_F(CommentTest, CommentWithNewlineAfter) {
    std::string input = "// Comment line\nx = 1;";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 1.0);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_SEMICOLON);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_EOF);
}

TEST_F(CommentTest, MultipleComments) {
    std::string input = "// First comment\n// Second comment\nx = 5;";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 5.0);
}

TEST_F(CommentTest, CommentInBetweenCode) {
    std::string input = "x = 1;\n// Comment in between\ny = 2;";
    Lexer lexer(input);
    
    // x = 1;
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 1.0);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_SEMICOLON);
    
    // Comment should be skipped, next token should be 'y'
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_IDENTIFIER);
    EXPECT_EQ(token5.value, "y");
}

TEST_F(CommentTest, CommentWithSpecialCharacters) {
    std::string input = "// Comment with symbols: +-*/(){}=<>!\nx = 42;";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 42.0);
}

TEST_F(CommentTest, DivisionVsComment) {
    // Test that division operator is not confused with comment
    std::string input = "x = 10 / 2;";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.numValue, 10.0);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_DIVIDE);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_NUMBER);
    EXPECT_EQ(token5.numValue, 2.0);
}

// Edge Case Tests
class EdgeCaseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EdgeCaseTest, EmptyInput) {
    std::string input = "";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, WhitespaceOnly) {
    std::string input = "   \t\n  \r\n  ";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, SingleCharacterTokens) {
    std::string input = "+";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_PLUS);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, SingleDigit) {
    std::string input = "5";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_NUMBER);
    EXPECT_EQ(token1.numValue, 5.0);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, SingleLetter) {
    std::string input = "x";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, VeryLongIdentifier) {
    std::string input = "very_long_identifier_name_with_underscores_123";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_IDENTIFIER);
    EXPECT_EQ(token.value, "very_long_identifier_name_with_underscores_123");
}

TEST_F(EdgeCaseTest, VeryLargeNumber) {
    std::string input = "123456789.987654321";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_DOUBLE_EQ(token.numValue, 123456789.987654321);
}

TEST_F(EdgeCaseTest, DecimalPointOnly) {
    std::string input = ".";
    Lexer lexer(input);
    
    // Decimal point alone should throw an exception or be treated as unknown
    EXPECT_THROW(lexer.getNextToken(), std::exception);
}

TEST_F(EdgeCaseTest, NumberWithLeadingZeros) {
    std::string input = "0001.2300";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_DOUBLE_EQ(token.numValue, 1.23);
}

TEST_F(EdgeCaseTest, ConsecutiveOperators) {
    std::string input = "+-*/";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_PLUS);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_MINUS);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_MULTIPLY);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_DIVIDE);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, MultiCharacterOperatorEdgeCases) {
    std::string input = ">= <= == != > < =";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_GTE);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_LTE);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_EQ);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_NEQ);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_GT);
    
    Token token6 = lexer.getNextToken();
    EXPECT_EQ(token6.type, TOK_LT);
    
    Token token7 = lexer.getNextToken();
    EXPECT_EQ(token7.type, TOK_ASSIGN);
    
    Token token8 = lexer.getNextToken();
    EXPECT_EQ(token8.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, CommentAtEndOfFileWithoutNewline) {
    std::string input = "x = 1; // comment at end";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_SEMICOLON);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, OnlySlashCharacter) {
    std::string input = "/";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_DIVIDE);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(EdgeCaseTest, IncompleteComment) {
    std::string input = "x = 1 /";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_ASSIGN);
    
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    
    Token token4 = lexer.getNextToken();
    EXPECT_EQ(token4.type, TOK_DIVIDE);
    
    Token token5 = lexer.getNextToken();
    EXPECT_EQ(token5.type, TOK_EOF);
}

// Parser Edge Case Tests
class ParserEdgeCaseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ParserEdgeCaseTest, EmptyInputParsing) {
    std::string input = "";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Empty input should throw an exception when trying to parse
    EXPECT_THROW(parser.parseStatement(), std::exception);
}

TEST_F(ParserEdgeCaseTest, OnlyWhitespaceParsing) {
    std::string input = "   \t\n  ";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Whitespace only should throw an exception when trying to parse
    EXPECT_THROW(parser.parseStatement(), std::exception);
}

TEST_F(ParserEdgeCaseTest, OnlyCommentParsing) {
    std::string input = "// This is just a comment";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Comment only should throw an exception when trying to parse
    EXPECT_THROW(parser.parseStatement(), std::exception);
}

TEST_F(ParserEdgeCaseTest, SingleNumberExpression) {
    std::string input = "42";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
}

TEST_F(ParserEdgeCaseTest, SingleVariableExpression) {
    std::string input = "x";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
}

TEST_F(ParserEdgeCaseTest, MinimalAssignment) {
    std::string input = "x=1";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    EXPECT_NE(stmt, nullptr);
}

TEST_F(ParserEdgeCaseTest, MinimalPrint) {
    std::string input = "print 1;";  // Add semicolon
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    EXPECT_NE(stmt, nullptr);
}

TEST_F(ParserEdgeCaseTest, NestedParentheses) {
    std::string input = "((((1))))";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
}

TEST_F(ParserEdgeCaseTest, MinimalIfStatement) {
    std::string input = "if(1){x=1;}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    EXPECT_NE(stmt, nullptr);
}

TEST_F(ParserEdgeCaseTest, MinimalWhileStatement) {
    std::string input = "while(1){x=1;}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    EXPECT_NE(stmt, nullptr);
}

TEST_F(ParserEdgeCaseTest, EmptyBlock) {
    std::string input = "if(1){}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    EXPECT_NE(stmt, nullptr);
}

TEST_F(ParserEdgeCaseTest, OperatorWithoutOperands) {
    std::string input = "+";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // This should fail gracefully or throw an exception
    EXPECT_THROW(parser.parse(), std::exception);
}

TEST_F(ParserEdgeCaseTest, UnmatchedOpenParenthesis) {
    std::string input = "(1 + 2";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // This should fail gracefully or throw an exception
    EXPECT_THROW(parser.parse(), std::exception);
}

TEST_F(ParserEdgeCaseTest, UnmatchedCloseParenthesis) {
    std::string input = "1 + 2)";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
    // Note: This might parse successfully as "1 + 2" and leave ")" unconsumed
}

TEST_F(ParserEdgeCaseTest, MissingSemicolon) {
    std::string input = "x = 1 y = 2";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt1 = parser.parseStatement();
    EXPECT_NE(stmt1, nullptr);
    
    // Second statement might fail without semicolon
    auto stmt2 = parser.parseStatement();
    // This behavior depends on parser implementation
}

TEST_F(ParserEdgeCaseTest, ChainedComparisons) {
    // This tests operator precedence edge cases
    std::string input = "1 < 2 < 3";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
    // This should parse as (1 < 2) < 3
}

TEST_F(ParserEdgeCaseTest, MaximumNestingDepth) {
    // Test deeply nested expressions
    std::string input = "1";
    for (int i = 0; i < 100; i++) {
        input = "(" + input + ")";
    }
    
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto expr = parser.parse();
    EXPECT_NE(expr, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}