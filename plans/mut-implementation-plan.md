# Implementation Plan

## Overview
Extend ArithLang to support mutable and immutable variable declarations using the `mut` keyword, implementing an immutable-by-default variable system with explicit mutability declarations.

This implementation adds a comprehensive variable mutability system to ArithLang that follows Rust-inspired immutable-by-default semantics. Variables will be immutable unless explicitly declared with the `mut` keyword, providing type safety and preventing accidental reassignment while allowing controlled mutation when needed. The system supports variable shadowing for immutable bindings and proper mutation tracking for mutable variables, all integrated seamlessly with the existing compiler pipeline including lexer, parser, type checker, and code generator.

## Types
Define mutability tracking data structures and enhanced symbol table entries.

```cpp
// Enhanced Symbol Table Entry (include/codegen.h)
struct Symbol {
    std::string name;
    llvm::Value* value;
    bool is_mutable;
    bool is_initialized;
    SourceLocation declaration_site;
    int scope_level;
    
    Symbol(const std::string& n, llvm::Value* v, bool mut = false) 
        : name(n), value(v), is_mutable(mut), is_initialized(true), scope_level(0) {}
};

// Variable Declaration Types (include/ast.h)
enum class AssignmentType {
    DECLARATION,     // First binding: x = 42 or mut x = 42
    REASSIGNMENT,    // Subsequent assignment: x = 43 (only valid for mutable)
    SHADOWING        // New immutable binding with same name
};
```

## Files
Modify existing compiler components to support mutability tracking and validation.

**Existing files to be modified:**
- `include/lexer.h` - Add TOK_MUT token type
- `src/lexer.cpp` - Add "mut" keyword recognition in handleKeywordOrIdentifier()
- `include/ast.h` - Enhance AssignmentExprAST with mutability flags
- `src/parser.cpp` - Modify parseAssignment() to handle mut keyword
- `include/codegen.h` - Enhance symbol table with mutability tracking
- `src/codegen.cpp` - Update variable storage and lookup with mutability checks
- `src/type_check.cpp` - Add mutability validation to assignment checking

**New files to be created:**
- `tests/test_mutability.cpp` - Comprehensive unit tests for mutability system
- `tests/k/test_mutable_basic.k` - Basic mutable variable functionality test
- `tests/k/test_immutable_error.k` - Immutable reassignment error test
- `tests/k/test_shadowing.k` - Variable shadowing behavior test
- `tests/k/test_mut_scope.k` - Scoped mutability test

**Configuration file updates:**
- `CMakeLists.txt` - Add new test targets for mutability tests

## Functions
Implement mutability checking and variable declaration enhancement across compiler phases.

**New functions:**
- `Lexer::handleMutKeyword()` in `src/lexer.cpp` - Handle "mut" keyword tokenization
- `Parser::parseMutDeclaration()` in `src/parser.cpp` - Parse mutable variable declarations
- `CodeGen::declareVariable()` in `src/codegen.cpp` - Enhanced variable declaration with mutability
- `CodeGen::canReassign()` in `src/codegen.cpp` - Check if variable can be reassigned
- `TypeChecker::validateAssignment()` in `src/type_check.cpp` - Validate assignment mutability rules

**Modified functions:**
- `Parser::parseAssignment()` - Enhanced to detect mut keyword and assignment type
- `CodeGen::getVariable()` - Updated to include mutability information
- `CodeGen::setVariable()` - Enhanced with mutability validation
- `TypeChecker::checkAssignment()` - Add mutability validation to existing type checking

## Classes
Enhance existing AST and symbol management classes with mutability support.

**Modified classes:**
- `AssignmentExprAST` in `include/ast.h` - Add is_mutable_declaration and assignment_type fields
- `CodeGen` in `include/codegen.h` - Enhanced symbol table with Symbol struct containing mutability info
- `Parser` in `include/parser.h` - Add private methods for mut parsing

**Enhanced class interfaces:**
```cpp
// Enhanced AssignmentExprAST
class AssignmentExprAST : public ExprAST {
private:
    std::string varName;
    std::unique_ptr<ExprAST> value;
    bool is_mutable_declaration;
    AssignmentType assignment_type;
    
public:
    AssignmentExprAST(const std::string& varName, std::unique_ptr<ExprAST> value, 
                      bool is_mut, AssignmentType type);
    bool isMutableDeclaration() const { return is_mutable_declaration; }
    AssignmentType getAssignmentType() const { return assignment_type; }
};
```

## Dependencies
No new external dependencies required - implementation uses existing LLVM and C++17 features.

All mutability tracking will be implemented using existing C++ standard library containers (std::map for symbol table, std::string for variable names) and LLVM Value system for variable storage. The implementation leverages the existing exception-based error handling system and integrates with the current ParseError framework for mutability violation reporting.

## Testing
Comprehensive test strategy covering positive and negative mutability scenarios.

**Unit test categories:**
- `MutabilityLexerTests` - Test mut keyword tokenization and recognition
- `MutabilityParserTests` - Test parsing of mutable declarations and assignments
- `MutabilityTypeCheckTests` - Test mutability validation in type checker
- `MutabilityCodeGenTests` - Test variable storage and retrieval with mutability

**Integration test scenarios:**
- Basic mutable variable declaration and mutation
- Immutable variable declaration and reassignment error
- Variable shadowing with mixed mutability
- Scope-based mutability behavior
- Complex expressions with mutable variables
- Error message quality for mutability violations

**Test file structure:**
```
tests/k/test_mutable_basic.k       - Basic functionality
tests/k/test_immutable_error.k     - Error cases  
tests/k/test_shadowing.k           - Shadowing behavior
tests/k/test_mut_scope.k           - Scoped mutability
```

## Implementation Order
Systematic implementation sequence to minimize integration conflicts and ensure comprehensive testing.

1. **Lexer Enhancement** - Add TOK_MUT token recognition and mut keyword handling
2. **AST Extension** - Enhance AssignmentExprAST with mutability flags and types  
3. **Parser Modification** - Update assignment parsing to handle mut declarations
4. **Symbol Table Enhancement** - Extend CodeGen symbol table with mutability tracking
5. **Type Checker Integration** - Add mutability validation to existing type checking
6. **Code Generation Update** - Implement mutability checks in variable operations
7. **Comprehensive Testing** - Create unit and integration tests for all scenarios
8. **Error Message Enhancement** - Implement clear mutability violation error messages
9. **Integration Validation** - Full compiler pipeline testing with mutability features
10. **Backward Compatibility Testing** - Ensure existing code continues to work correctly
