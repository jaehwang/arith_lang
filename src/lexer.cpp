#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input) : input(input), pos(0) {
    currentChar = pos < input.length() ? input[pos] : '\0';
}

void Lexer::advance() {
    pos++;
    currentChar = pos < input.length() ? input[pos] : '\0';
}

void Lexer::skipWhitespace() {
    while (currentChar != '\0' && std::isspace(currentChar)) {
        advance();
    }
}

double Lexer::readNumber() {
    std::string numStr;
    
    while (currentChar != '\0' && (std::isdigit(currentChar) || currentChar == '.')) {
        numStr += currentChar;
        advance();
    }
    
    return std::stod(numStr);
}

std::string Lexer::readIdentifier() {
    std::string identifier;
    
    while (currentChar != '\0' && (std::isalnum(currentChar) || currentChar == '_')) {
        identifier += currentChar;
        advance();
    }
    
    return identifier;
}

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (currentChar == '\0') {
        return Token(TOK_EOF);
    }
    
    if (std::isdigit(currentChar)) {
        double value = readNumber();
        return Token(TOK_NUMBER, std::to_string(value), value);
    }
    
    if (std::isalpha(currentChar) || currentChar == '_') {
        std::string identifier = readIdentifier();
        if (identifier == "print") {
            return Token(TOK_PRINT, identifier);
        } else if (identifier == "if") {
            return Token(TOK_IF, identifier);
        } else if (identifier == "else") {
            return Token(TOK_ELSE, identifier);
        } else if (identifier == "while") {
            return Token(TOK_WHILE, identifier);
        }
        return Token(TOK_IDENTIFIER, identifier);
    }
    
    char ch = currentChar;
    advance();
    
    switch (ch) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')':
        case ';':
        case '{':
        case '}':
            return Token(static_cast<TokenType>(ch), std::string(1, ch));
        case '=':
            if (currentChar == '=') {
                advance();
                return Token(TOK_EQ, "==");
            }
            return Token(TOK_ASSIGN, "=");
        case '>':
            if (currentChar == '=') {
                advance();
                return Token(TOK_GTE, ">=");
            }
            return Token(TOK_GT, ">");
        case '<':
            if (currentChar == '=') {
                advance();
                return Token(TOK_LTE, "<=");
            }
            return Token(TOK_LT, "<");
        case '!':
            if (currentChar == '=') {
                advance();
                return Token(TOK_NEQ, "!=");
            }
            throw std::runtime_error("Unknown character: !");
        default:
            throw std::runtime_error("Unknown character: " + std::string(1, ch));
    }
}