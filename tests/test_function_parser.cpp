#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "function_ast.h"

class FunctionParserTest : public ::testing::Test {};

// Helper: parse input, assert program with one statement, return the statement.
static ASTNode* parseOneStatement(const std::string& input,
                                  std::unique_ptr<ASTNode>& programOwner) {
    Lexer lexer(input);
    Parser parser(lexer);
    programOwner = parser.parseProgram();
    auto* prog = dynamic_cast<ProgramAST*>(programOwner.get());
    if (!prog || prog->getStatements().size() != 1) return nullptr;
    return prog->getStatements()[0].get();
}

TEST_F(FunctionParserTest, ExpressionFunctionNoParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn() => 42;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->getVarName(), "f");

    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_TRUE(fnLit->isExpressionFunction());
    EXPECT_EQ(fnLit->getParams().size(), 0u);
    EXPECT_NE(fnLit->getBody(), nullptr);
}

TEST_F(FunctionParserTest, ExpressionFunctionOneParam) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("double = fn(x) => x + x;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_TRUE(fnLit->isExpressionFunction());
    ASSERT_EQ(fnLit->getParams().size(), 1u);
    EXPECT_EQ(fnLit->getParams()[0].name, "x");
    EXPECT_FALSE(fnLit->getParams()[0].is_mutable);
}

TEST_F(FunctionParserTest, ExpressionFunctionMultipleParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("add = fn(x, y) => x + y;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->getVarName(), "add");

    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_TRUE(fnLit->isExpressionFunction());
    ASSERT_EQ(fnLit->getParams().size(), 2u);
    EXPECT_EQ(fnLit->getParams()[0].name, "x");
    EXPECT_EQ(fnLit->getParams()[1].name, "y");
    EXPECT_FALSE(fnLit->getParams()[0].is_mutable);
    EXPECT_FALSE(fnLit->getParams()[1].is_mutable);
}

TEST_F(FunctionParserTest, ExpressionFunctionThreeParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(a, b, c) => a + b + c;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_TRUE(fnLit->isExpressionFunction());
    ASSERT_EQ(fnLit->getParams().size(), 3u);
    EXPECT_EQ(fnLit->getParams()[0].name, "a");
    EXPECT_EQ(fnLit->getParams()[1].name, "b");
    EXPECT_EQ(fnLit->getParams()[2].name, "c");
}

TEST_F(FunctionParserTest, TrailingCommaIsParseError) {
    Lexer lexer("f = fn(x,) => x;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

TEST_F(FunctionParserTest, TrailingCommaMultiParamIsParseError) {
    Lexer lexer("f = fn(x, y,) => x + y;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

TEST_F(FunctionParserTest, MissingArrowIsParseError) {
    Lexer lexer("f = fn(x) x;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

TEST_F(FunctionParserTest, MissingParenIsParseError) {
    Lexer lexer("f = fn x) => x;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

TEST_F(FunctionParserTest, CaptureListIsEmpty) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x) => x;", owner);
    ASSERT_NE(stmt, nullptr);
    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_EQ(fnLit->getCaptures().size(), 0u);
}
