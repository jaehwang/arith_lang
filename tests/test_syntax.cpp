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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}