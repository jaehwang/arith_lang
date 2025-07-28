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