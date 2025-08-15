#include "parse_error_reporting.h"
#include <cstdio>

std::string getLine(const std::string& source, int lineNumber) {
    if (lineNumber < 1) return "";
    int currentLine = 1;
    size_t lineStart = 0;
    const size_t n = source.size();
    for (size_t i = 0; i < n && currentLine < lineNumber; ++i) {
        if (source[i] == '\n') {
            currentLine++;
            lineStart = i + 1;
        }
    }
    if (currentLine != lineNumber) return "";
    size_t lineEnd = source.find('\n', lineStart);
    if (lineEnd == std::string::npos) lineEnd = n;
    return source.substr(lineStart, lineEnd - lineStart);
}

void printParseError(const ParseError& e, const std::string& source) {
    const auto& L = e.loc;
    std::fprintf(stderr, "%s:%d:%d: error: %s\n",
                 L.file.c_str(), L.line, L.column, e.what());

    const auto lineStr = getLine(source, L.line);
    if (!lineStr.empty()) {
        std::fprintf(stderr, "%s\n", lineStr.c_str());
        std::string caret;
        for (int i = 1; i < L.column; ++i) caret.push_back(' ');
        caret.push_back('^');
        std::fprintf(stderr, "%s\n", caret.c_str());
    }
}
