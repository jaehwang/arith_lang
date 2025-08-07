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
    
    // Parse entire program with two statements
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 2);
}

TEST_F(SyntaxTest, VariableAssignmentAndUsage) {
    std::string input = "y = 42; print y;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire program with two statements
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 2);
}

TEST_F(SyntaxTest, ComplexExpression) {
    std::string input = "result = (10 + 5) * 2 - 3; print result;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire program with two statements
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 2);
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
    
    // Empty input should return empty program
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 0);
}

TEST_F(ParserEdgeCaseTest, OnlyWhitespaceParsing) {
    std::string input = "   \t\n  ";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Whitespace only should return empty program
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 0);
}

TEST_F(ParserEdgeCaseTest, OnlyCommentParsing) {
    std::string input = "// This is just a comment";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Comment only should return empty program
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 0);
}

TEST_F(ParserEdgeCaseTest, MinimalAssignment) {
    std::string input = "x=1";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, MinimalPrint) {
    std::string input = "print 1;";  // Add semicolon
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, NestedParentheses) {
    std::string input = "((((1))))";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, MinimalIfStatement) {
    std::string input = "if(1){x=1;}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, MinimalWhileStatement) {
    std::string input = "while(1){x=1;}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, EmptyBlock) {
    std::string input = "if(1){}";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

TEST_F(ParserEdgeCaseTest, MissingSemicolon) {
    std::string input = "x = 1 y = 2";
    Lexer lexer(input);
    Parser parser(lexer);
    
    // Parse entire program - should handle missing semicolon gracefully
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    // Cast to ProgramAST to check statements
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_GE(programAST->getStatements().size(), 1); // Should parse at least one statement
}

TEST_F(ParserEdgeCaseTest, ChainedComparisons) {
    // This tests operator precedence edge cases
    std::string input = "1 < 2 < 3;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    // This should parse as (1 < 2) < 3
}

TEST_F(ParserEdgeCaseTest, MaximumNestingDepth) {
    // Test deeply nested expressions
    std::string input = "1";
    for (int i = 0; i < 100; i++) {
        input = "(" + input + ")";
    }
    input += ";";  // Add semicolon to make it a valid statement
    
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    EXPECT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
}

// Unary Minus Tests
class UnaryMinusTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UnaryMinusTest, BasicUnaryMinus) {
    std::string input = "print -5.0;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is a PrintStmtAST
    auto* printStmt = dynamic_cast<PrintStmtAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(printStmt, nullptr);
    
    // Check that the expression is a UnaryExprAST
    auto* unaryExpr = dynamic_cast<UnaryExprAST*>(printStmt->getExpr());
    ASSERT_NE(unaryExpr, nullptr);
    EXPECT_EQ(unaryExpr->getOperator(), '-');
    
    // Check that the operand is a NumberExprAST
    auto* numberExpr = dynamic_cast<NumberExprAST*>(unaryExpr->getOperand());
    ASSERT_NE(numberExpr, nullptr);
    EXPECT_DOUBLE_EQ(numberExpr->getValue(), 5.0);
}

TEST_F(UnaryMinusTest, UnaryMinusAssignment) {
    std::string input = "x = -3.14;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is an AssignmentExprAST
    auto* assignExpr = dynamic_cast<AssignmentExprAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(assignExpr, nullptr);
    EXPECT_EQ(assignExpr->getVarName(), "x");
    
    // Check that the value is a UnaryExprAST
    auto* unaryExpr = dynamic_cast<UnaryExprAST*>(assignExpr->getValue());
    ASSERT_NE(unaryExpr, nullptr);
    EXPECT_EQ(unaryExpr->getOperator(), '-');
}

TEST_F(UnaryMinusTest, UnaryMinusVariable) {
    std::string input = "y = -x;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is an AssignmentExprAST
    auto* assignExpr = dynamic_cast<AssignmentExprAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(assignExpr, nullptr);
    
    // Check that the value is a UnaryExprAST with a variable operand
    auto* unaryExpr = dynamic_cast<UnaryExprAST*>(assignExpr->getValue());
    ASSERT_NE(unaryExpr, nullptr);
    EXPECT_EQ(unaryExpr->getOperator(), '-');
    
    // Check that the operand is a VariableExprAST
    auto* varExpr = dynamic_cast<VariableExprAST*>(unaryExpr->getOperand());
    ASSERT_NE(varExpr, nullptr);
    EXPECT_EQ(varExpr->getName(), "x");
}

TEST_F(UnaryMinusTest, UnaryMinusParentheses) {
    std::string input = "result = -(2.0 + 3.0);";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is an AssignmentExprAST
    auto* assignExpr = dynamic_cast<AssignmentExprAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(assignExpr, nullptr);
    
    // Check that the value is a UnaryExprAST
    auto* unaryExpr = dynamic_cast<UnaryExprAST*>(assignExpr->getValue());
    ASSERT_NE(unaryExpr, nullptr);
    EXPECT_EQ(unaryExpr->getOperator(), '-');
    
    // Check that the operand is a BinaryExprAST (2.0 + 3.0)
    auto* binaryExpr = dynamic_cast<BinaryExprAST*>(unaryExpr->getOperand());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->getOperator(), '+');
}

TEST_F(UnaryMinusTest, DoubleUnaryMinus) {
    std::string input = "z = --5.0;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is an AssignmentExprAST
    auto* assignExpr = dynamic_cast<AssignmentExprAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(assignExpr, nullptr);
    
    // Check that the value is a UnaryExprAST (outer minus)
    auto* outerUnary = dynamic_cast<UnaryExprAST*>(assignExpr->getValue());
    ASSERT_NE(outerUnary, nullptr);
    EXPECT_EQ(outerUnary->getOperator(), '-');
    
    // Check that the operand is also a UnaryExprAST (inner minus)
    auto* innerUnary = dynamic_cast<UnaryExprAST*>(outerUnary->getOperand());
    ASSERT_NE(innerUnary, nullptr);
    EXPECT_EQ(innerUnary->getOperator(), '-');
    
    // Check that the final operand is a NumberExprAST
    auto* numberExpr = dynamic_cast<NumberExprAST*>(innerUnary->getOperand());
    ASSERT_NE(numberExpr, nullptr);
    EXPECT_DOUBLE_EQ(numberExpr->getValue(), 5.0);
}

TEST_F(UnaryMinusTest, UnaryMinusInBinaryExpression) {
    std::string input = "result = -2.0 * 3.0;";
    Lexer lexer(input);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    ASSERT_NE(program, nullptr);
    
    auto* programAST = dynamic_cast<ProgramAST*>(program.get());
    ASSERT_NE(programAST, nullptr);
    EXPECT_EQ(programAST->getStatements().size(), 1);
    
    // Check that the statement is an AssignmentExprAST
    auto* assignExpr = dynamic_cast<AssignmentExprAST*>(programAST->getStatements()[0].get());
    ASSERT_NE(assignExpr, nullptr);
    
    // Check that the value is a BinaryExprAST
    auto* binaryExpr = dynamic_cast<BinaryExprAST*>(assignExpr->getValue());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->getOperator(), '*');
    
    // Check that the left operand is a UnaryExprAST
    auto* unaryExpr = dynamic_cast<UnaryExprAST*>(binaryExpr->getLHS());
    ASSERT_NE(unaryExpr, nullptr);
    EXPECT_EQ(unaryExpr->getOperator(), '-');
    
    // Check that the right operand is a NumberExprAST
    auto* numberExpr = dynamic_cast<NumberExprAST*>(binaryExpr->getRHS());
    ASSERT_NE(numberExpr, nullptr);
    EXPECT_DOUBLE_EQ(numberExpr->getValue(), 3.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}