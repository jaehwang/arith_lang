# PRD: Defining Functions in ArithLang

## Introduction

Add first-class function definitions and lexical closures to ArithLang. Users will be able to define functions as values (assigned to variables), call them, capture outer variables, and pass functions as arguments to other functions. This brings ArithLang from a simple scripting language to one that supports functional programming patterns.

The feature is grounded in the existing implementation plan at `plans/sample-function-implementation-plan.md`.

---

## Goals

- Allow users to define named functions using `fn(params) => expr` (expression) and `fn(params) { block }` (block) syntax
- Allow functions to capture enclosing variables lexically (closures), immutably by default
- Allow explicit mutable capture of outer `mut` variables via `mut(var1, var2)` capture clause
- Support function calls with `f(arg1, arg2)` syntax
- Support higher-order functions (functions passed as arguments and returned as values)
- Support recursion through self-referencing bindings
- Emit correct LLVM IR for all function and closure constructs
- Maintain consistent, high-quality error reporting for all new syntax errors

---

## User Stories

### US-001: Tokenize function keywords and operators ✅ **COMPLETED**
**Description:** As a developer, I need the lexer to recognize `fn`, `return`, and `=>` so the parser can handle function syntax.

**Acceptance Criteria:**
- [x] `fn` tokenized as `TOK_FN`
- [x] `return` tokenized as `TOK_RETURN`
- [x] `=>` tokenized as `TOK_ARROW`
- [x] Existing tokens and keywords are unaffected
- [x] Build passes (`cmake --build build`)
- [x] Unit tests pass (`cd build && ctest`)

---

### US-002: Define AST nodes for functions ✅ **COMPLETED**
**Description:** As a developer, I need AST node classes for function literals, function calls, and return statements so the compiler pipeline can represent functions structurally.

**Acceptance Criteria:**
- [x] `FunctionLiteralAST` stores: parameter list (`FunctionParameter`), capture list (`CapturedVariable`), body (`ASTNode`), expression-vs-block flag, source location
- [x] `FunctionCallAST` stores: callee expression, argument list, source location
- [x] `ReturnStmtAST` stores: optional return value, source location
- [x] Supporting structs `FunctionParameter`, `CapturedVariable`, `ClosureContext`, `FunctionValue` are defined
- [x] Build passes

---

### US-003: Parse expression-style function literals ✅ **COMPLETED**
**Description:** As a user, I want to write `add = fn(x, y) => x + y;` so I can define simple single-expression functions.

**Acceptance Criteria:**
- [x] `fn(params) => expr` parsed into `FunctionLiteralAST` with `is_expression_function = true`
- [x] Parameters parsed as a comma-separated list inside `()`
- [x] Trailing comma in parameter list is a parse error with source location
- [x] Build passes; unit tests for expression function parsing pass

---

### US-004: Parse block-style function literals ✅ **COMPLETED**
**Description:** As a user, I want to write `fn(x) { result = x * 2; return result; }` so I can define multi-statement functions.

**Acceptance Criteria:**
- [x] `fn(params) { statements }` parsed into `FunctionLiteralAST` with `is_expression_function = false`
- [x] `return expr;` inside a block parsed as `ReturnStmtAST`
- [x] `return` at top level (outside any function) is a parse error with source location
- [x] Build passes; unit tests for block function parsing pass

---

### US-005: Parse mutable parameter declarations ✅ **COMPLETED**
**Description:** As a user, I want to write `fn(mut x, y) => x + y` so parameters can be declared mutable and reassigned inside the function body.

**Acceptance Criteria:**
- [x] `mut` before a parameter name sets `FunctionParameter::is_mutable = true`
- [x] Non-`mut` parameters are immutable; reassigning them inside the body is a type-check error
- [x] Build passes; unit tests cover mutable and immutable parameter parsing

---

### US-006: Parse capture clause for closures ✅ **COMPLETED**
**Description:** As a user, I want to write `fn() mut(counter) { counter = counter + 1; return counter; }` so I can explicitly capture outer mutable variables.

**Acceptance Criteria:**
- [x] `mut(var1, var2)` after the parameter list parsed as a capture clause
- [x] Each named variable must exist in the outer scope; capturing an undeclared variable is a parse/type-check error
- [x] Capturing an immutable outer variable with `mut(...)` is a type-check error
- [x] Build passes; unit tests cover capture clause parsing

---

### US-007: Parse function call expressions ✅ **COMPLETED**
**Description:** As a user, I want to write `add(1, 2)` to call a function stored in a variable.

**Acceptance Criteria:**
- [x] `expr(args)` parsed as `FunctionCallAST` in postfix expression position
- [x] Calling a non-function value (e.g., `x = 42; x(10);`) is a type-check error
- [x] Wrong argument count (e.g., `add(1)` when `add` takes 2) is a type-check error
- [x] Build passes; unit tests for function call parsing pass

---

### US-008: Type-check functions and calls ✅ **COMPLETED**
**Description:** As a developer, I need the type checker to validate function definitions and call sites so users get clear errors before codegen.

**Acceptance Criteria:**
- [x] `validateFunctionLiteral` checks: no duplicate parameter names, capture variables exist and are mutable when in `mut(...)` clause
- [x] `validateFunctionCall` checks: callee is a function value, argument count matches parameter count
- [x] `validateReturnStatement` checks: `return` is inside a function body
- [x] All errors use existing GCC-style `ParseError` with `SourceLocation`
- [x] Build passes; unit tests for each validation method pass

---

### US-009: Generate LLVM IR for expression functions ✅ **COMPLETED**
**Description:** As a user, I want `add = fn(x, y) => x + y; print add(3, 4);` to compile and print `7.000000000000000`.

**Acceptance Criteria:**
- [x] `generateFunctionLiteral` emits a named `llvm::Function` for expression-body functions
- [x] `generateFunctionCall` emits a `call` instruction
- [x] Integration test `tests/k/function_basic.k` passes via `./test_runner.sh`
- [x] Build passes; codegen unit tests pass

---

### US-010: Generate LLVM IR for block functions with return ✅ **COMPLETED**
**Description:** As a user, I want block-style functions with explicit `return` to compile and execute correctly.

**Acceptance Criteria:**
- [x] Block function body generates sequential LLVM IR with a `ret` at `return` statement
- [x] Expression-function's implicit return value is the expression result
- [x] Integration test with block function passes via `./test_runner.sh`
- [x] Build passes; codegen unit tests pass

---

### US-011: Implement immutable closures ✅ **COMPLETED**
**Description:** As a user, I want `scale = fn(x) => x * multiplier;` to capture `multiplier` from the outer scope by value.

**Acceptance Criteria:**
- [x] Variables referenced inside a function body but not in the parameter list are captured by value at definition time
- [x] Captured value is frozen — subsequent changes to the outer variable do not affect the closure
- [x] Integration test `tests/k/function_closure.k` passes via `./test_runner.sh`
- [x] Build passes

---

### US-012: Implement mutable closures ✅ **COMPLETED**
**Description:** As a user, I want `increment = fn() mut(counter) { counter = counter + 1; return counter; };` to modify and persist the outer `counter` across calls.

**Acceptance Criteria:**
- [x] Mutable captures use `std::shared_ptr<double>` so the closure and outer scope share the same value
- [x] Multiple calls to the closure reflect accumulated state
- [x] Integration test `tests/k/function_mutable_capture.k` passes:
  ```
  // EXPECTED: 1.000000000000000
  // EXPECTED: 2.000000000000000
  ```
- [x] Build passes

---

### US-013: Support higher-order functions ✅ **COMPLETED**
**Description:** As a user, I want to pass functions as arguments and return functions as values so I can write composable code like `apply(square, 5)`.

**Acceptance Criteria:**
- [x] Functions can be passed as arguments and stored in variables like any other value
- [x] `generateFunctionCall` handles callee that is itself the result of a function call
- [x] Integration test `tests/k/function_higher_order.k` passes:
  ```
  // EXPECTED: 25.000000000000000
  ```
- [x] Build passes

---

### US-014: Support recursive functions ✅ **COMPLETED**
**Description:** As a user, I want to write a recursive function (e.g., factorial) using self-referencing variable bindings.

**Acceptance Criteria:**
- [x] A function stored in variable `f` can call `f(...)` inside its own body
- [x] Integration test `tests/k/function_recursive.k` passes (e.g., recursive factorial of 5 = 120)
- [x] Build passes

---

### US-015: Add integration test files ✅ **COMPLETED**
**Description:** As a developer, I need integration test `.k` files with `// EXPECTED:` annotations for all key function scenarios.

**Acceptance Criteria:**
- [x] `tests/k/function_basic.k` — simple expression function call
- [x] `tests/k/function_closure.k` — immutable capture
- [x] `tests/k/function_mutable_capture.k` — mutable capture with state
- [x] `tests/k/function_higher_order.k` — passing function as argument
- [x] `tests/k/function_recursive.k` — recursive function
- [x] All files pass `./test_runner.sh` from project root

---

### US-016: Update syntax specification ✅ **COMPLETED**
**Description:** As a developer or user reading the docs, I need the language syntax spec to reflect the new function grammar.

**Acceptance Criteria:**
- [x] `specs/syntax.md` BNF updated to include `<function_literal>`, `<call_expr>`, `<return_stmt>`, `<capture_clause>`
- [x] Examples of expression function, block function, closure, and higher-order function added to the spec
- [x] `README.md` features list updated to mention function definitions and closures

---

## Functional Requirements ✅ **ALL COMPLETED**

- **FR-1:** ✅ The lexer must recognize `fn`, `return`, and `=>` as distinct tokens.
- **FR-2:** ✅ A function literal `fn(p1, p2) => expr` is a valid primary expression assignable to any variable.
- **FR-3:** ✅ A function literal `fn(p1, p2) { stmts }` is a valid primary expression; the last executed `return expr;` provides the function's value.
- **FR-4:** ✅ Parameters are immutable by default; `mut p` makes a parameter mutable within the function body.
- **FR-5:** ✅ A function literal may include a capture clause `mut(v1, v2)` listing outer `mut` variables to capture by shared reference.
- **FR-6:** ✅ Variables referenced inside a function body that are not parameters and not in the capture clause are captured by value (immutable snapshot).
- **FR-7:** ✅ A function call `f(arg1, arg2)` is a valid postfix expression evaluated left-to-right.
- **FR-8:** Calling a non-function value is a compile-time type-check error.
- **FR-9:** Calling a function with the wrong number of arguments is a compile-time type-check error.
- **FR-10:** `return` outside any function body is a compile-time error.
- **FR-11:** All functions operate on `double` values, consistent with the existing type system.
- **FR-12:** All errors use existing GCC-style source location reporting.

---

## Non-Goals (Out of Scope)

- No explicit type annotations (e.g., `fn(x: double) -> double`)
- No named function declarations (e.g., `fn add(x, y) { ... }` at statement level) — functions are always anonymous literals assigned to variables
- No module import/export of functions
- No variadic functions
- No tail-call optimization
- No async/generator functions
- No garbage collection for closures (assume programs are short-lived)

---

## Technical Considerations

- **New files:** `include/function_ast.h`, `src/function_codegen.cpp`, `tests/test_function_parser.cpp`, `tests/test_function_codegen.cpp`, `tests/test_closures.cpp`
- **Modified files:** `include/lexer.h`, `src/lexer.cpp`, `include/ast.h`, `src/parser.cpp`, `include/parser.h`, `src/type_check.cpp`, `include/type_check.h`, `src/codegen.cpp`, `include/codegen.h`, `CMakeLists.txt`
- **LLVM types used:** `llvm::FunctionType`, `llvm::Function::Create`, `llvm::StructType` (for closure context), `llvm::CallInst`, `llvm::ReturnInst`
- **Mutable capture memory model:** `std::shared_ptr<double>` shared between closure and outer scope
- **Closure context struct:** LLVM struct holding captured value slots, passed as an implicit extra argument to the closure's LLVM function
- **Scope management:** `CodeGen` must track `current_function` and `in_function_context` to distinguish top-level vs. function-body code paths
- **Implementation order:** Follow the phased plan in `plans/sample-function-implementation-plan.md` — Lexer → AST → Parser → Type Checker → CodeGen → Closures → Tests → Docs

---

## Success Metrics ✅ **ALL ACHIEVED**

- ✅ All existing integration tests in `tests/k/` continue to pass after the feature is added (37/37 passing)
- ✅ All 6 new function integration tests pass via `./test_runner.sh`
  - `function_basic.k` ✅
  - `function_closure.k` ✅
  - `function_mutable_capture.k` ✅
  - `function_higher_order.k` ✅
  - `function_recursive.k` ✅
  - `function_block.k` ✅
- ✅ All new unit tests pass via `cd build && ctest` (11/11 test suites passing)
- ✅ A user can write recursive factorial, higher-order `map`/`apply`, and counter closures in `.k` files
- ✅ Error messages for invalid function syntax, wrong arity, and bad captures show accurate source line/column

---

## Implementation Complete - 2026-03-04

**All user stories (US-001 through US-016) have been successfully implemented and tested.**

### Technical Implementation Summary
- **New files created:** ✅ `include/function_ast.h`, `src/function_codegen.cpp`, `tests/test_function_parser.cpp`
- **Modified files:** ✅ All compiler pipeline files updated (lexer, parser, type_check, codegen, CMakeLists.txt)
- **LLVM integration:** ✅ Complete with `llvm::FunctionType`, `llvm::StructType` for closures, `llvm::CallInst`
- **Memory model:** ✅ `std::shared_ptr<double>` for mutable captures, value capture for immutable
- **Scope management:** ✅ Function context tracking in CodeGen

### Resolved Open Questions
- ✅ **Self-referential functions:** Immutable bindings work for recursion - `f` is in scope when function body executes
- ✅ **Capture list size:** No enforced maximum - handled naturally by implementation
- ✅ **Mutable capture requirement:** Only `mut`-declared variables can be in `mut(...)` capture clause (as planned)

---

## Non-Goals (Out of Scope)

- No explicit type annotations (e.g., `fn(x: double) -> double`)
- No named function declarations (e.g., `fn add(x, y) { ... }` at statement level) — functions are always anonymous literals assigned to variables
- No module import/export of functions
- No variadic functions
- No tail-call optimization
- No async/generator functions
- No garbage collection for closures (assume programs are short-lived)
