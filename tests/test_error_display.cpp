#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "parse_error_reporting.h"

using ::testing::HasSubstr;

TEST(ErrorDisplayTest, BasicFormat) {
    SourceLocation loc{"test.k", 1, 5};
    ParseError error("Expected ';' after print statement", loc);
    testing::internal::CaptureStderr();
    printParseError(error, "print 42");
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_THAT(output, HasSubstr("test.k:1:5: error: Expected ';' after print statement"));
}

TEST(ErrorDisplayTest, WithSourceSnippet) {
    SourceLocation loc{"test.k", 1, 8};
    ParseError error("Expected ';' after print statement", loc);
    testing::internal::CaptureStderr();
    printParseError(error, "print 42");
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_THAT(output, HasSubstr("print 42"));
    EXPECT_THAT(output, HasSubstr("       ^"));
}

TEST(ErrorDisplayTest, MultipleLines) {
    std::string source = "x = 1;\n123 = 42;\ny = 3;";
    SourceLocation loc{"test.k", 2, 3};
    ParseError error("Invalid assignment target", loc);
    testing::internal::CaptureStderr();
    printParseError(error, source);
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_THAT(output, HasSubstr("test.k:2:3: error: Invalid assignment target"));
    EXPECT_THAT(output, HasSubstr("123 = 42;"));
    EXPECT_THAT(output, HasSubstr("  ^"));
}

// getLine helper tests
TEST(GetLineTest, FirstLine) {
    std::string source = "line1\nline2\nline3";
    EXPECT_EQ(getLine(source, 1), "line1");
}

TEST(GetLineTest, MiddleLine) {
    std::string source = "line1\nline2\nline3";
    EXPECT_EQ(getLine(source, 2), "line2");
}

TEST(GetLineTest, LastLine) {
    std::string source = "line1\nline2\nline3";
    EXPECT_EQ(getLine(source, 3), "line3");
}

TEST(GetLineTest, InvalidLineNumber) {
    std::string source = "line1\nline2";
    EXPECT_EQ(getLine(source, 0), "");
    EXPECT_EQ(getLine(source, -1), "");
    EXPECT_EQ(getLine(source, 5), "");
}

TEST(GetLineTest, EmptySource) {
    std::string source = "";
    EXPECT_EQ(getLine(source, 1), "");
}

TEST(GetLineTest, OnlyNewlines) {
    std::string source = "\n\n\n";
    EXPECT_EQ(getLine(source, 1), "");
    EXPECT_EQ(getLine(source, 2), "");
    EXPECT_EQ(getLine(source, 3), "");
}
