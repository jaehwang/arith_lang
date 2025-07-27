#pragma once
#include <string>
#include <memory>

enum TokenType {
    TOK_EOF = -1,
    TOK_NUMBER = -2,
    TOK_IDENTIFIER = -3,
    TOK_PRINT = -4,
    TOK_PLUS = '+',
    TOK_MINUS = '-',
    TOK_MULTIPLY = '*',
    TOK_DIVIDE = '/',
    TOK_LPAREN = '(',
    TOK_RPAREN = ')',
    TOK_SEMICOLON = ';',
    TOK_ASSIGN = '='
};

struct Token {
    TokenType type;
    std::string value;
    double numValue;
    
    Token(TokenType t, const std::string& v = "", double n = 0.0) 
        : type(t), value(v), numValue(n) {}
};

class Lexer {
private:
    std::string input;
    size_t pos;
    char currentChar;
    
    void advance();
    void skipWhitespace();
    double readNumber();
    std::string readIdentifier();
    
public:
    Lexer(const std::string& input);
    Token getNextToken();
    bool isAtEnd() const { return pos >= input.length(); }
};