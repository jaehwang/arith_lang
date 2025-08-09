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

void Lexer::skipComment() {
    if (currentChar == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
        // // 주석: 라인 끝까지 건너뛰기
        while (currentChar != '\0' && currentChar != '\n') {
            advance();
        }
        // 개행 문자도 건너뛰기
        if (currentChar == '\n') {
            advance();
        }
    }
}

double Lexer::readNumber() {
    std::string numStr;
    bool hasDot = false;
    
    // Read digits and at most one decimal point
    while (currentChar != '\0' && (std::isdigit(currentChar) || currentChar == '.')) {
        if (currentChar == '.') {
            if (hasDot) {
                // Multiple decimal points - invalid number
                throw std::runtime_error("Invalid number format: multiple decimal points");
            }
            hasDot = true;
        }
        numStr += currentChar;
        advance();
    }
    
    // Check for invalid formats like ending with dot
    if (numStr.empty() || numStr.back() == '.') {
        throw std::runtime_error("Invalid number format: number cannot end with decimal point");
    }
    
    // Check for starting with dot
    if (numStr.front() == '.') {
        throw std::runtime_error("Invalid number format: number cannot start with decimal point");
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

std::string Lexer::readString() {
    std::string str;
    advance(); // consume opening quote
    
    while (currentChar != '\0' && currentChar != '"') {
        if (currentChar == '\\') {
            advance(); // consume backslash
            if (currentChar == '\0') {
                throw std::runtime_error("Unterminated string literal");
            }
            
            // Handle escape sequences
            switch (currentChar) {
                case 'n':
                    str += '\n';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '"':
                    str += '"';
                    break;
                default:
                    throw std::runtime_error("Invalid escape sequence in string literal");
            }
        } else {
            str += currentChar;
        }
        advance();
    }
    
    if (currentChar != '"') {
        throw std::runtime_error("Unterminated string literal");
    }
    
    advance(); // consume closing quote
    return str;
}

Token Lexer::handleKeywordOrIdentifier(const std::string& identifier) {
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

Token Lexer::handleOperator(char ch) {
    advance();
    
    switch (ch) {
        case '+': case '-': case '*': case '/':
        case '(': case ')': case '{': case '}':
        case ';': case ',':
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

Token Lexer::getNextToken() {
    while (true) {
        skipWhitespace();
        
        if (currentChar == '\0') {
            return Token(TOK_EOF);
        }
        
        // 주석 확인 및 처리
        if (currentChar == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
            skipComment();
            continue; // 주석을 건너뛰고 다시 토큰 찾기
        }
        
        break; // 주석이 아니면 토큰 처리 진행
    }
    
    if (std::isdigit(currentChar)) {
        double value = readNumber();
        return Token(TOK_NUMBER, std::to_string(value), value);
    }
    
    if (std::isalpha(currentChar) || currentChar == '_') {
        std::string identifier = readIdentifier();
        return handleKeywordOrIdentifier(identifier);
    }
    
    if (currentChar == '"') {
        std::string str = readString();
        return Token(TOK_STRING, str);
    }
    
    return handleOperator(currentChar);
}