#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "lexer.h"
#include "parser.h"

using ::testing::HasSubstr;

TEST(ParserErrorTest, ErrorHere_MissingSemicolonAfterPrint) {
    std::string source = "print 42"; // missing semicolon
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for missing semicolon";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Expected ';'"));
        EXPECT_EQ(e.loc.line, 1);
        EXPECT_EQ(e.loc.column, 9); // EOF position after "print 42"
    }
}

TEST(ParserErrorTest, ErrorAt_UnmatchedParen) {
    std::string source = "x = (10 + 5;";
    Lexer lexer(source, "test.k");
    Parser parser(lexer);
    try {
        parser.parseProgram();
        FAIL() << "Expected ParseError for unmatched parenthesis";
    } catch (const ParseError& e) {
        EXPECT_THAT(e.what(), HasSubstr("Expected ')'"));
        // Implementation-specific exact column isn't asserted here.
    }
}
