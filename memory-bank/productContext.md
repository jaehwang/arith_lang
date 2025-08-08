# Product Context: ArithLang

## Problem Statement
Traditional programming education often glosses over how high-level code becomes executable machine instructions. Students learn syntax and algorithms but miss the crucial transformation process that makes their code run on actual hardware.

## Solution
ArithLang serves as an educational bridge, demonstrating how a programming language is implemented from scratch. It provides:

1. **Tangible Learning**: See how mathematical expressions become machine code
2. **LLVM Education**: Learn modern compiler infrastructure
3. **Phase Separation**: Understand lexing → parsing → AST → code generation pipeline
4. **Immediate Feedback**: Write expressions and see immediate LLVM IR output

## Target Users

### Primary: CS Students & Educators
- Students taking compiler courses
- Educators teaching programming language implementation
- Self-learners exploring compiler construction

### Secondary: Developers
- Developers curious about LLVM
- Engineers working with DSLs (Domain Specific Languages)
- Anyone interested in language implementation

## User Experience Goals

### Simplicity First
- Write mathematical expressions naturally: `x = 5 + 3 * 2`
- Use familiar C-style printf formatting: `print "Value: %.2f", x;`
- Output string literals and formatted text: `print "Hello, World!";`
- See immediate results: both execution and generated IR
- Clear error messages when syntax is wrong

### Educational Value
- Generated LLVM IR should be readable and instructive
- Code structure should demonstrate best practices
- Each compiler phase should be clearly separated and understandable

### Development Experience
- Easy to build and run
- Comprehensive test suite for confidence
- Clear documentation for extending the language

## Success Metrics
1. **Functionality**: All defined language constructs work correctly ✅
2. **Code Quality**: Clean, well-tested, maintainable codebase ✅
3. **Educational Value**: Code serves as good example of compiler construction ✅
4. **Usability**: Easy to build, run, and experiment with ✅
5. **Scientific Accuracy**: Supports high-precision mathematical calculations ✅

## Real-World Validation
The compiler has been successfully validated with practical mathematical computation:

**Pi Calculation Implementation**:
- **Algorithm**: Leibniz formula (π/4 = 1 - 1/3 + 1/5 - 1/7 + ...)
- **Complexity**: 10,000 iterations with multiple variable updates per loop
- **Result**: `3.141492653590034` (accurate to ~5 decimal places with current iteration count)
- **Demonstrates**: 
  - Complex iterative algorithms
  - High-precision floating point arithmetic
  - Scientific computation capabilities
  - Compiler's real-world applicability beyond toy examples

## Language Specification
- **Grammar Definition**: Complete BNF grammar specification available in `syntax.md`
- **Print Statement Specification**: Comprehensive printf-like functionality detailed in `print.md`
- **Formal Syntax**: Defines all language constructs including:
  - Arithmetic expressions with proper operator precedence
  - Variable assignment and usage
  - Control flow (if/else, while loops)
  - Advanced print statements with string literals and format specifiers
  - Block statements with proper scoping
- **Implementation Reference**: Parser implementation follows the grammar rules exactly

## Key Differentiators
- **LLVM Focus**: Uses industry-standard backend instead of custom VM
- **Incremental Complexity**: Starts simple, can be extended step by step
- **Real Execution**: Not just interpretation—actual compiled code execution
- **Modern C++**: Demonstrates contemporary C++ practices in systems programming
- **Formal Grammar**: Well-defined BNF syntax specification for clear language semantics
