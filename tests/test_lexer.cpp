#include <gtest/gtest.h>
#include "lexer.h"

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

TEST_F(CommentTest, CommentAtEndOfFileWithoutNewline) {
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

// Lexer Edge Case Tests
class LexerEdgeCaseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(LexerEdgeCaseTest, EmptyInput) {
    std::string input = "";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, WhitespaceOnly) {
    std::string input = "   \t\n  \r\n  ";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, SingleCharacterTokens) {
    std::string input = "+";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_PLUS);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, SingleDigit) {
    std::string input = "5";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_NUMBER);
    EXPECT_EQ(token1.numValue, 5.0);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, SingleLetter) {
    std::string input = "x";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_IDENTIFIER);
    EXPECT_EQ(token1.value, "x");
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, VeryLongIdentifier) {
    std::string input = "very_long_identifier_name_with_underscores_123";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_IDENTIFIER);
    EXPECT_EQ(token.value, "very_long_identifier_name_with_underscores_123");
}

TEST_F(LexerEdgeCaseTest, VeryLargeNumber) {
    std::string input = "123456789.987654321";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_DOUBLE_EQ(token.numValue, 123456789.987654321);
}

TEST_F(LexerEdgeCaseTest, DecimalPointOnly) {
    std::string input = ".";
    Lexer lexer(input);
    
    // Decimal point alone should throw an exception or be treated as unknown
    EXPECT_THROW(lexer.getNextToken(), std::exception);
}

TEST_F(LexerEdgeCaseTest, NumberWithLeadingZeros) {
    std::string input = "0001.2300";
    Lexer lexer(input);
    
    Token token = lexer.getNextToken();
    EXPECT_EQ(token.type, TOK_NUMBER);
    EXPECT_DOUBLE_EQ(token.numValue, 1.23);
}

TEST_F(LexerEdgeCaseTest, ConsecutiveOperators) {
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

TEST_F(LexerEdgeCaseTest, MultiCharacterOperatorEdgeCases) {
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

TEST_F(LexerEdgeCaseTest, OnlySlashCharacter) {
    std::string input = "/";
    Lexer lexer(input);
    
    Token token1 = lexer.getNextToken();
    EXPECT_EQ(token1.type, TOK_DIVIDE);
    
    Token token2 = lexer.getNextToken();
    EXPECT_EQ(token2.type, TOK_EOF);
}

TEST_F(LexerEdgeCaseTest, IncompleteComment) {
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}