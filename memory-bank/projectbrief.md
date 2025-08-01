# Project Brief: ArithLang

## Overview
ArithLang is an LLVM-based arithmetic expression compiler that transforms simple mathematical expressions and basic programming constructs into executable machine code. This is a learning/demonstration project showcasing compiler construction using modern C++ and LLVM infrastructure.

## Core Requirements

### Language Features
- **Arithmetic Operations**: +, -, *, / with proper operator precedence
- **Comparison Operations**: >, <, >=, <=, ==, !=
- **Variables**: Assignment and reference (x = 42)
- **Control Flow**: 
  - Conditional statements (if-else)
  - Loop constructs (while)
- **I/O**: Print statements for output (15-digit precision)
- **Comments**: Line comments using // syntax
- **Data Types**: Floating-point numbers (double precision)

### Technical Goals
1. **LLVM Integration**: Generate LLVM IR code for all language constructs
2. **Execution**: Support both IR generation and direct execution
3. **Testing**: Comprehensive test suite covering all components
4. **Clean Architecture**: Separate lexing, parsing, AST generation, and code generation phases

## Project Scope

### In Scope
- Simple arithmetic compiler with basic control flow
- LLVM backend for code generation
- Interactive execution capability
- Comprehensive testing framework

### Out of Scope
- Complex data types (arrays, structures)
- Function definitions and calls
- Advanced language features (classes, modules)
- Optimization passes
- Error recovery mechanisms

## Implementation Constraints

### Output Precision
- **Print Statement Precision**: Fixed at 15 decimal places using "%.15f" format
- **Rationale**: Matches IEEE 754 double precision effective digits (~15-17)
- **Trade-off**: Higher precision output vs. configurable precision complexity
- **Future Enhancement**: Could be extended to support configurable precision

## Success Criteria
1. Successfully compile and execute arithmetic expressions
2. Support all defined language constructs
3. Generate valid LLVM IR
4. Maintain clean, testable codebase
5. Comprehensive test coverage across all components

## Architecture Philosophy
- **Modularity**: Clear separation between compiler phases
- **Testability**: Each component independently testable
- **Simplicity**: Focus on core compiler concepts without unnecessary complexity
- **Educational Value**: Code should be readable and demonstrate compiler construction principles
