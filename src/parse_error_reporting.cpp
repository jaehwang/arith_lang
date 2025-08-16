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
    // Split e.what() by newlines to support multi-line diagnostics
    std::string what = e.what();
    std::vector<std::string> lines;
    size_t start = 0;
    while (true) {
        size_t pos = what.find('\n', start);
        if (pos == std::string::npos) {
            lines.push_back(what.substr(start));
            break;
        }
        lines.push_back(what.substr(start, pos - start));
        start = pos + 1;
    }

    // Header: first line as the primary error message
    const char* header = lines.empty() ? what.c_str() : lines[0].c_str();
    std::fprintf(stderr, "%s:%d:%d: error: %s\n",
                 L.file.c_str(), L.line, L.column, header);

    // Source preview with caret under the offending column
    const auto lineStr = getLine(source, L.line);
    if (!lineStr.empty()) {
        std::fprintf(stderr, "%s\n", lineStr.c_str());
        std::string caret;
        for (int i = 1; i < L.column; ++i) caret.push_back(' ');
        caret.push_back('^');
        std::fprintf(stderr, "%s\n", caret.c_str());
    }

    // Print any additional diagnostic lines (notes, help)
    for (size_t i = 1; i < lines.size(); ++i) {
        if (!lines[i].empty()) {
            std::fprintf(stderr, "%s\n", lines[i].c_str());
        } else {
            std::fprintf(stderr, "\n");
        }
    }
}
