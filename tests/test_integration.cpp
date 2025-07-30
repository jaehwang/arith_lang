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
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(IntegrationTest, PrintStatement) {
    std::string input = "print 42;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(IntegrationTest, IfStatementParsing) {
    std::string input = "if (x > 0) { y = 1; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(IntegrationTest, WhileLoopParsing) {
    std::string input = "while (n > 0) { n = n - 1; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(IntegrationTest, CompleteProgram) {
    std::string input = "x = 5; y = x * 2; if (y > 8) { print y; }";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire program as single AST
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 3);
}

TEST_F(IntegrationTest, CommentsIgnoredInParsing) {
    std::string input = "// This is a comment\nx = 1; // Another comment\nprint x;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire program with comments
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 2);
}

TEST_F(IntegrationTest, FactorialProgram) {
    std::string input = "n = 5; result = 1; while (n > 0) { result = result * n; n = n - 1; } print result;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire factorial program as single AST
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_GE(programAST->getStatements().size(), 4); // Should have at least 4 statements
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}