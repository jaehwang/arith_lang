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
        }
        return Token(TOK_IDENTIFIER, identifier);
    }
    
    TokenType tokenType = static_cast<TokenType>(currentChar);
    advance();
    
    switch (tokenType) {
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_MULTIPLY:
        case TOK_DIVIDE:
        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_SEMICOLON:
        case TOK_ASSIGN:
            return Token(tokenType, std::string(1, static_cast<char>(tokenType)));
        default:
            throw std::runtime_error("Unknown character: " + std::string(1, static_cast<char>(tokenType)));
    }
}