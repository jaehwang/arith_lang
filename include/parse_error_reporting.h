#pragma once
#include <string>
#include "parser.h" // for ParseError and SourceLocation

// Extract the 1-based line from the source; returns empty string if invalid
std::string getLine(const std::string& source, int lineNumber);

// Print standardized parse error with optional source snippet and caret to stderr
void printParseError(const ParseError& e, const std::string& source);
