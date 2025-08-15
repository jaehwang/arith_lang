#include "lexer.h"
#include "parser.h" // For ParseError
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input, std::string filename)
    : input(input), filename(std::move(filename)), pos(0), currentChar('\0'), line(1), column(1) {
    currentChar = pos < input.length() ? input[pos] : '\0';
}

void Lexer::advance() {
    // handle CRLF and LF
    if (currentChar == '\r') {
        // if next char is \n, consume both but count as single newline
        pos++;
        if (pos < input.length() && input[pos] == '\n') {
            pos++;
        }
        line += 1;
        column = 1;
    } else if (currentChar == '\n') {
        pos++;
        line += 1;
        column = 1;
    } else if (currentChar == '\0') {
        // already at end
    } else {
        pos++;
        column += 1;
    }
    currentChar = pos < input.length() ? input[pos] : '\0';
}

void Lexer::skipWhitespace() {
    while (currentChar != '\0' && std::isspace(static_cast<unsigned char>(currentChar))) {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
        // // 주석: 라인 끝까지 건너뛰기
        while (currentChar != '\0' && currentChar != '\n' && currentChar != '\r') {
            advance();
        }
        // 개행까지 소비 (advance가 줄바꿈 처리)
        if (currentChar == '\n' || currentChar == '\r') {
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
    
    while (currentChar != '\0' && (std::isalnum(static_cast<unsigned char>(currentChar)) || currentChar == '_' || (static_cast<unsigned char>(currentChar) >= 0x80))) {
        identifier += currentChar;
        advance();
    }
    
    return identifier;
}

std::string Lexer::readString() {
    std::string str;
    advance(); // consume opening quote
    
    while (true) {
        // Unterminated if we hit end-of-file or newline before closing quote
        if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
            throw ParseError("Unterminated string literal", currentLocation());
        }
        
        if (currentChar == '"') {
            break; // closing quote found
        }
        
        if (currentChar == '\\') {
            advance(); // consume backslash
            if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
                throw ParseError("Unterminated string literal", currentLocation());
            }
            switch (currentChar) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default:
                    throw ParseError("Invalid escape sequence in string literal", currentLocation());
            }
        } else {
            str += currentChar;
        }
        advance();
    }
    
    advance(); // consume closing quote
    return str;
}

Token Lexer::handleKeywordOrIdentifier(const std::string& identifier, const SourceRange& r) {
    if (identifier == "print") {
        return Token(TOK_PRINT, identifier, 0.0, r);
    } else if (identifier == "if") {
        return Token(TOK_IF, identifier, 0.0, r);
    } else if (identifier == "else") {
        return Token(TOK_ELSE, identifier, 0.0, r);
    } else if (identifier == "while") {
        return Token(TOK_WHILE, identifier, 0.0, r);
    }
    return Token(TOK_IDENTIFIER, identifier, 0.0, r);
}

Token Lexer::handleOperator(char ch, const SourceLocation& startLoc) {
    // we will advance as we consume
    SourceLocation curStart = startLoc;
    advance();
    
    switch (ch) {
        case '+': case '-': case '*': case '/':
        case '(': case ')': case '{': case '}':
        case ';': case ',':
            return Token(static_cast<TokenType>(ch), std::string(1, ch), 0.0,
                         SourceRange{curStart, currentLocation()});
        case '=':
            if (currentChar == '=') {
                advance();
                return Token(TOK_EQ, "==", 0.0, SourceRange{curStart, currentLocation()});
            }
            return Token(TOK_ASSIGN, "=", 0.0, SourceRange{curStart, currentLocation()});
        case '>':
            if (currentChar == '=') {
                advance();
                return Token(TOK_GTE, ">=", 0.0, SourceRange{curStart, currentLocation()});
            }
            return Token(TOK_GT, ">", 0.0, SourceRange{curStart, currentLocation()});
        case '<':
            if (currentChar == '=') {
                advance();
                return Token(TOK_LTE, "<=", 0.0, SourceRange{curStart, currentLocation()});
            }
            return Token(TOK_LT, "<", 0.0, SourceRange{curStart, currentLocation()});
        case '!':
            if (currentChar == '=') {
                advance();
                return Token(TOK_NEQ, "!=", 0.0, SourceRange{curStart, currentLocation()});
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
            auto loc = currentLocation();
            return Token(TOK_EOF, "", 0.0, SourceRange{loc, loc});
        }
        
        // 주석 확인 및 처리
        if (currentChar == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
            skipComment();
            continue; // 주석을 건너뛰고 다시 토큰 찾기
        }
        
        break; // 주석이 아니면 토큰 처리 진행
    }
    // record start location of the token
    SourceLocation startLoc = currentLocation();
    
    if (std::isdigit(static_cast<unsigned char>(currentChar))) {
        double value = readNumber();
        return Token(TOK_NUMBER, std::to_string(value), value, SourceRange{startLoc, currentLocation()});
    }
    
    if (std::isalpha(static_cast<unsigned char>(currentChar)) || currentChar == '_' || (static_cast<unsigned char>(currentChar) >= 0x80)) {
        std::string identifier = readIdentifier();
        return handleKeywordOrIdentifier(identifier, SourceRange{startLoc, currentLocation()});
    }
    
    if (currentChar == '"') {
        try {
            std::string str = readString();
            return Token(TOK_STRING, str, 0.0, SourceRange{startLoc, currentLocation()});
        } catch (const std::runtime_error& e) {
            // Report lexer string errors at the current position (e.g., end of line for unterminated)
            throw ParseError(e.what(), currentLocation());
        }
    }
    
    return handleOperator(currentChar, startLoc);
}