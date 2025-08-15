#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lexer.h"
#include "parser.h"

using ::testing::HasSubstr;

// 1.1 SourceLocation Basic Tests
TEST(SourceLocationTest, DefaultConstruction) {
    SourceLocation loc;
    EXPECT_EQ(loc.file, "");
    EXPECT_EQ(loc.line, 1);
    EXPECT_EQ(loc.column, 1);
}

TEST(SourceLocationTest, CustomConstruction) {
    SourceLocation loc{"test.k", 5, 10};
    EXPECT_EQ(loc.file, "test.k");
    EXPECT_EQ(loc.line, 5);
    EXPECT_EQ(loc.column, 10);
}

// 1.2 SourceRange Tests
TEST(SourceRangeTest, SingleCharacterToken) {
    SourceLocation start{"test.k", 1, 5};
    SourceLocation end{"test.k", 1, 6};
    SourceRange range{start, end};
    EXPECT_EQ(range.start.column, 5);
    EXPECT_EQ(range.end.column, 6);
    EXPECT_EQ(range.end.column - range.start.column, 1);
}

TEST(SourceRangeTest, MultiCharacterToken) {
    SourceLocation start{"test.k", 1, 10};
    SourceLocation end{"test.k", 1, 15};  // 5-character token
    SourceRange range{start, end};
    EXPECT_EQ(range.end.column - range.start.column, 5);
}

// 1.3 Token Range Tests
TEST(TokenTest, WithSourceRange) {
    SourceRange range{{"test.k", 1, 5}, {"test.k", 1, 7}};
    Token token(TOK_NUMBER, "42", 42.0, range);
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 5);
    EXPECT_EQ(token.range.end.column, 7);
}

TEST(TokenTest, DefaultRange) {
    Token token(TOK_PLUS, "+");
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 1);
}

// 1.4 Lexer Location Tracking Tests
TEST(LexerLocationTest, BasicLocationTracking) {
    Lexer lexer("42", "test.k");
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_EQ(token.range.start.file, "test.k");
    EXPECT_EQ(token.range.start.line, 1);
    EXPECT_EQ(token.range.start.column, 1);
    EXPECT_EQ(token.range.end.column, 3);
}

TEST(LexerLocationTest, MultipleTokensOnSameLine) {
    Lexer lexer("x + 42", "test.k");
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.range.start.column, 1);
    EXPECT_EQ(token1.range.end.column, 2);
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_PLUS);
    EXPECT_EQ(token2.range.start.column, 3);
    EXPECT_EQ(token2.range.end.column, 4);
    Token token3 = lexer.getNextToken();
    EXPECT_EQ(token3.type, TOK_NUMBER);
    EXPECT_EQ(token3.range.start.column, 5);
    EXPECT_EQ(token3.range.end.column, 7);
}

TEST(LexerLocationTest, NewlineHandling) {
    Lexer lexer("x\ny", "test.k");
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.range.start.line, 1);
    EXPECT_EQ(token1.range.start.column, 1);
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.range.start.line, 2);
    EXPECT_EQ(token2.range.start.column, 1);
}

TEST(LexerLocationTest, TabHandling) {
    Lexer lexer("x\ty", "test.k");
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.range.start.column, 1);
    Token token2 = lexer.getNextToken();
    // tab counts as 1 column; 'y' starts after 1 tab and one char => column 3
    EXPECT_EQ(token2.range.start.column, 3);
}

TEST(LexerLocationTest, MultiCharacterOperators) {
    Lexer lexer("x >= y", "test.k");
    lexer.getNextToken(); // 'x'
    Token token = lexer.getNextToken(); // '>='
    EXPECT_EQ(token.type, TOK_GTE);
    EXPECT_EQ(token.range.start.column, 3);
    EXPECT_EQ(token.range.end.column, 5);
}

TEST(LexerLocationTest, StringLiteralWithEscapes) {
    Lexer lexer("\"hello\\nworld\"", "test.k");
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_STRING);
    EXPECT_EQ(token.range.start.column, 1);
    EXPECT_EQ(token.range.end.column, 15);
}

TEST(LexerLocationTest, EOFToken) {
    Lexer lexer("x", "test.k");
    lexer.getNextToken();
    Token eofToken = lexer.getNextToken();
    EXPECT_EQ(eofToken.type, TOK_EOF);
    EXPECT_EQ(eofToken.range.start.column, 2);
    EXPECT_EQ(eofToken.range.start.column, eofToken.range.end.column);
}

// 1.5 ParseError Exception Tests
TEST(ParseErrorTest, BasicConstruction) {
    SourceLocation loc{"test.k", 5, 10};
    ParseError error("Test message", loc);
    EXPECT_STREQ(error.what(), "Test message");
    EXPECT_EQ(error.loc.file, "test.k");
    EXPECT_EQ(error.loc.line, 5);
    EXPECT_EQ(error.loc.column, 10);
}

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
