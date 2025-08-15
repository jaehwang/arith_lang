#pragma once
#include <string>
#include <memory>

// Source location and range (1-based indices)
struct SourceLocation {
    std::string file;
    int line = 1;
    int column = 1;
};

struct SourceRange {
    SourceLocation start;
    SourceLocation end; // exclusive end
};

enum TokenType {
    TOK_EOF = -1,
    TOK_NUMBER = -2,
    TOK_IDENTIFIER = -3,
    TOK_PRINT = -4,
    TOK_IF = -5,
    TOK_ELSE = -6,
    TOK_WHILE = -7,
    TOK_STRING = -8,
    TOK_PLUS = '+',
    TOK_MINUS = '-',
    TOK_MULTIPLY = '*',
    TOK_DIVIDE = '/',
    TOK_LPAREN = '(',
    TOK_RPAREN = ')',
    TOK_SEMICOLON = ';',
    TOK_ASSIGN = '=',
    TOK_LBRACE = '{',
    TOK_RBRACE = '}',
    TOK_COMMA = ',',
    TOK_GT = '>',
    TOK_LT = '<',
    TOK_GTE = -9,
    TOK_LTE = -10,
    TOK_EQ = -11,
    TOK_NEQ = -12
};

struct Token {
    TokenType type;
    std::string value;
    double numValue;
    SourceRange range;
    
    Token(TokenType t, const std::string& v = "", double n = 0.0, SourceRange r = {}) 
        : type(t), value(v), numValue(n), range(std::move(r)) {}
};

class Lexer {
private:
    std::string input;
    std::string filename;
    size_t pos;
    char currentChar;
    int line = 1;
    int column = 1; // 1-based
    
    void advance();
    void skipWhitespace();
    void skipComment();
    double readNumber();
    std::string readIdentifier();
    std::string readString();
    Token handleKeywordOrIdentifier(const std::string& identifier, const SourceRange& range);
    Token handleOperator(char ch, const SourceLocation& startLoc);
    
    inline SourceLocation currentLocation() const { return SourceLocation{filename, line, column}; }
    
public:
    Lexer(const std::string& input, std::string filename = "<stdin>");
    Token getNextToken();
    bool isAtEnd() const { return pos >= input.length(); }
    const std::string& getFilename() const { return filename; }
};