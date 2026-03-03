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

// ---- Block-style function literal tests (US-004) ----

TEST_F(FunctionParserTest, BlockFunctionIsNotExpressionFunction) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x) { y = x * 2; };", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_FALSE(fnLit->isExpressionFunction());
    ASSERT_EQ(fnLit->getParams().size(), 1u);
    EXPECT_EQ(fnLit->getParams()[0].name, "x");
    EXPECT_NE(fnLit->getBody(), nullptr);
}

TEST_F(FunctionParserTest, BlockFunctionBodyIsBlock) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x) { y = x * 2; };", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);

    auto* block = dynamic_cast<BlockAST*>(fnLit->getBody());
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->getStatements().size(), 1u);
}

TEST_F(FunctionParserTest, BlockFunctionWithReturnStatement) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x) { return x * 2; };", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_FALSE(fnLit->isExpressionFunction());

    auto* block = dynamic_cast<BlockAST*>(fnLit->getBody());
    ASSERT_NE(block, nullptr);
    ASSERT_EQ(block->getStatements().size(), 1u);

    auto* ret = dynamic_cast<ReturnStmtAST*>(block->getStatements()[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_TRUE(ret->hasValue());
}

TEST_F(FunctionParserTest, BlockFunctionMultipleStatements) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x, y) { a = x + y; return a; };", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);

    auto* block = dynamic_cast<BlockAST*>(fnLit->getBody());
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->getStatements().size(), 2u);

    auto* ret = dynamic_cast<ReturnStmtAST*>(block->getStatements()[1].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_TRUE(ret->hasValue());
}

TEST_F(FunctionParserTest, BlockFunctionNoParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn() { return 42; };", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_EQ(fnLit->getParams().size(), 0u);
    EXPECT_FALSE(fnLit->isExpressionFunction());
}

TEST_F(FunctionParserTest, ReturnAtTopLevelIsParseError) {
    Lexer lexer("return 42;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

TEST_F(FunctionParserTest, MissingBraceOrArrowAfterParamsIsError) {
    Lexer lexer("f = fn(x) x;");
    Parser parser(lexer);
    EXPECT_THROW(parser.parseProgram(), ParseError);
}

// ---- Mutable parameter declaration tests (US-005) ----

TEST_F(FunctionParserTest, AllImmutableParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x, y) => x + y;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getParams().size(), 2u);
    EXPECT_FALSE(fnLit->getParams()[0].is_mutable);
    EXPECT_FALSE(fnLit->getParams()[1].is_mutable);
}

TEST_F(FunctionParserTest, AllMutableParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(mut x, mut y) => x + y;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getParams().size(), 2u);
    EXPECT_EQ(fnLit->getParams()[0].name, "x");
    EXPECT_TRUE(fnLit->getParams()[0].is_mutable);
    EXPECT_EQ(fnLit->getParams()[1].name, "y");
    EXPECT_TRUE(fnLit->getParams()[1].is_mutable);
}

TEST_F(FunctionParserTest, MixedMutableImmutableParams) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(x, mut y) => x + y;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getParams().size(), 2u);
    EXPECT_EQ(fnLit->getParams()[0].name, "x");
    EXPECT_FALSE(fnLit->getParams()[0].is_mutable);
    EXPECT_EQ(fnLit->getParams()[1].name, "y");
    EXPECT_TRUE(fnLit->getParams()[1].is_mutable);
}

TEST_F(FunctionParserTest, SingleMutableParam) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn(mut n) => n;", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getParams().size(), 1u);
    EXPECT_EQ(fnLit->getParams()[0].name, "n");
    EXPECT_TRUE(fnLit->getParams()[0].is_mutable);
}

// ---- Capture clause tests (US-006) ----

TEST_F(FunctionParserTest, NoCaptureClauseGivesEmptyCaptures) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn() { return 1; };", owner);
    ASSERT_NE(stmt, nullptr);
    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    EXPECT_EQ(fnLit->getCaptures().size(), 0u);
}

TEST_F(FunctionParserTest, SingleCaptureClause) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn() mut(counter) { return counter; };", owner);
    ASSERT_NE(stmt, nullptr);
    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getCaptures().size(), 1u);
    EXPECT_EQ(fnLit->getCaptures()[0].name, "counter");
    EXPECT_TRUE(fnLit->getCaptures()[0].is_mutable_capture);
}

TEST_F(FunctionParserTest, MultipleCaptureClause) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("f = fn() mut(a, b, c) { return a; };", owner);
    ASSERT_NE(stmt, nullptr);
    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);
    auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue());
    ASSERT_NE(fnLit, nullptr);
    ASSERT_EQ(fnLit->getCaptures().size(), 3u);
    EXPECT_EQ(fnLit->getCaptures()[0].name, "a");
    EXPECT_EQ(fnLit->getCaptures()[1].name, "b");
    EXPECT_EQ(fnLit->getCaptures()[2].name, "c");
    EXPECT_TRUE(fnLit->getCaptures()[0].is_mutable_capture);
    EXPECT_TRUE(fnLit->getCaptures()[1].is_mutable_capture);
    EXPECT_TRUE(fnLit->getCaptures()[2].is_mutable_capture);
}

// ---- Function call expression tests (US-007) ----

TEST_F(FunctionParserTest, FunctionCallZeroArgs) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("result = f();", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* call = dynamic_cast<FunctionCallAST*>(assign->getValue());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->getArgs().size(), 0u);

    auto* callee = dynamic_cast<VariableExprAST*>(call->getCallee());
    ASSERT_NE(callee, nullptr);
    EXPECT_EQ(callee->getName(), "f");
}

TEST_F(FunctionParserTest, FunctionCallOneArg) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("result = f(42);", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* call = dynamic_cast<FunctionCallAST*>(assign->getValue());
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->getArgs().size(), 1u);

    auto* callee = dynamic_cast<VariableExprAST*>(call->getCallee());
    ASSERT_NE(callee, nullptr);
    EXPECT_EQ(callee->getName(), "f");
}

TEST_F(FunctionParserTest, FunctionCallMultipleArgs) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("result = f(1, 2, 3);", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* call = dynamic_cast<FunctionCallAST*>(assign->getValue());
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->getArgs().size(), 3u);

    auto* callee = dynamic_cast<VariableExprAST*>(call->getCallee());
    ASSERT_NE(callee, nullptr);
    EXPECT_EQ(callee->getName(), "f");
}

TEST_F(FunctionParserTest, FunctionCallNested) {
    std::unique_ptr<ASTNode> owner;
    auto* stmt = parseOneStatement("result = f(g(x));", owner);
    ASSERT_NE(stmt, nullptr);

    auto* assign = dynamic_cast<AssignmentExprAST*>(stmt);
    ASSERT_NE(assign, nullptr);

    auto* outerCall = dynamic_cast<FunctionCallAST*>(assign->getValue());
    ASSERT_NE(outerCall, nullptr);
    ASSERT_EQ(outerCall->getArgs().size(), 1u);

    auto* outerCallee = dynamic_cast<VariableExprAST*>(outerCall->getCallee());
    ASSERT_NE(outerCallee, nullptr);
    EXPECT_EQ(outerCallee->getName(), "f");

    auto* innerCall = dynamic_cast<FunctionCallAST*>(outerCall->getArgs()[0].get());
    ASSERT_NE(innerCall, nullptr);
    ASSERT_EQ(innerCall->getArgs().size(), 1u);

    auto* innerCallee = dynamic_cast<VariableExprAST*>(innerCall->getCallee());
    ASSERT_NE(innerCallee, nullptr);
    EXPECT_EQ(innerCallee->getName(), "g");
}
