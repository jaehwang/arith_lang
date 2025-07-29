#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(IntegrationTest, VariableAssignmentAndUsage) {
    std::string input = "x = 42;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    ASSERT_NE(stmt, nullptr);
}

TEST_F(IntegrationTest, PrintStatement) {
    std::string input = "print 42;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    ASSERT_NE(stmt, nullptr);
}

TEST_F(IntegrationTest, IfStatementParsing) {
    std::string input = "if (x > 0) { y = 1; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    ASSERT_NE(stmt, nullptr);
}

TEST_F(IntegrationTest, WhileLoopParsing) {
    std::string input = "while (n > 0) { n = n - 1; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto stmt = parser.parseStatement();
    ASSERT_NE(stmt, nullptr);
}

TEST_F(IntegrationTest, CompleteProgram) {
    std::string input = "x = 5; y = x * 2; if (y > 8) { print y; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse first statement: x = 5
    auto stmt1 = parser.parseStatement();
    ASSERT_NE(stmt1, nullptr);
    
    // Parse second statement: y = x * 2
    auto stmt2 = parser.parseStatement();
    ASSERT_NE(stmt2, nullptr);
    
    // Parse third statement: if (y > 8) { print y; }
    auto stmt3 = parser.parseStatement();
    ASSERT_NE(stmt3, nullptr);
}

TEST_F(IntegrationTest, CommentsIgnoredInParsing) {
    std::string input = "// This is a comment\nx = 1; // Another comment\nprint x;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse first statement: x = 1
    auto stmt1 = parser.parseStatement();
    ASSERT_NE(stmt1, nullptr);
    
    // Parse second statement: print x
    auto stmt2 = parser.parseStatement();
    ASSERT_NE(stmt2, nullptr);
}

TEST_F(IntegrationTest, FactorialProgram) {
    std::string input = "n = 5; result = 1; while (n > 0) { result = result * n; n = n - 1; } print result;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse all statements
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    try {
        while (true) {
            auto stmt = parser.parseStatement();
            if (!stmt) break;
            statements.push_back(std::move(stmt));
        }
    } catch (const std::exception& e) {
        // Expected when reaching EOF
    }
    
    EXPECT_GE(statements.size(), 4); // Should have at least 4 statements
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}