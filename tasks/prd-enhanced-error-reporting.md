# PRD: Enhanced Error Reporting System for ArithLang

## Introduction

Improve ArithLang's error diagnostic system from basic single-error reporting to a modern, developer-friendly multi-error system with rich context, color output, smart suggestions, and IDE-integration readiness. The current system reports only the first error with basic `file:line:col: error: message` format. This enhancement will significantly improve developer experience, reduce debugging time, and prepare the language for IDE integration.

The feature is grounded in the detailed implementation plan at `plans/error-reporting-improvement-plan.md`.

---

## Goals

- Implement multi-error collection: report multiple errors in a single compilation run (up to configurable limit)
- Add error recovery mechanism: parser continues after syntax errors to find additional issues
- Provide rich error context: show source code snippets with precise highlighting using carets/tildes
- Support color output: ANSI color codes for terminals with automatic detection and `--no-color` flag
- Add smart suggestions: "did you mean?" for typos, common mistake detection with fix hints
- Implement diagnostic severity levels: Error, Warning, Info, Hint
- Prepare LSP-compatible structure: structured diagnostics with ranges for future IDE integration
- Maintain full backward compatibility: existing tests continue to work with updated error format

---

## User Stories

### US-001: Define diagnostic data structures
**Description:** As a developer, I need structured diagnostic types to represent errors with severity, location ranges, and related messages.

**Acceptance Criteria:**
- [ ] `DiagnosticSeverity` enum defined (Error, Warning, Info, Hint)
- [ ] `SourcePosition` struct with line/column
- [ ] `SourceRange` struct with filename, start position, end position
- [ ] `DiagnosticMessage` struct for related messages with range
- [ ] `Diagnostic` class with severity, code, message, primary range, related messages, suggestions
- [ ] Factory methods: `SourceRange::fromLocation()`, `SourceRange::fromToken()`
- [ ] Build passes (`cmake --build build`)

---

### US-002: Implement DiagnosticEngine for error collection
**Description:** As a developer, I need a centralized diagnostic engine that collects errors instead of throwing on first error.

**Acceptance Criteria:**
- [ ] `DiagnosticEngine` class stores vector of diagnostics
- [ ] Methods: `report()`, `reportError()`, `reportWarning()`, `reportInfo()`
- [ ] Tracks error count, warning count, fatal error flag
- [ ] `max_errors` parameter (default 10) prevents infinite error loops
- [ ] `hasErrors()`, `hasWarnings()`, `hasFatalError()` status methods
- [ ] `getDiagnostics()` returns const reference to collected diagnostics
- [ ] `clear()` method for resetting state
- [ ] Build passes with new files: `include/diagnostic.h`, `src/diagnostic.cpp`

---

### US-003: Implement SourceManager for context display
**Description:** As a developer, I need a source manager that loads file contents and provides line access for context display.

**Acceptance Criteria:**
- [ ] `SourceManager` class with file contents cache (map: filename → vector of lines)
- [ ] `loadFile(filename)` method reads and stores file by lines
- [ ] `getLine(filename, line)` returns optional single line
- [ ] `getLines(filename, start_line, end_line)` returns line range
- [ ] `getFileContents(filename)` returns entire file as line vector
- [ ] Handles missing files gracefully (returns empty optional)
- [ ] Build passes with new files: `include/source_manager.h`, `src/source_manager.cpp`

---

### US-004: Implement ColorManager for terminal output
**Description:** As a user, I want colored error output in the terminal for better readability.

**Acceptance Criteria:**
- [ ] `ColorManager` class with ANSI color code support
- [ ] `Color` enum (Reset, Bold, Red, Green, Yellow, Blue, Magenta, Cyan, White)
- [ ] `colorize(text, color, bold=false)` applies ANSI codes
- [ ] `shouldEnableColor()` static method detects TTY, checks `NO_COLOR` and `CI` env vars
- [ ] `setEnabled(bool)` allows manual control
- [ ] Color codes are not applied when `color_enabled = false`
- [ ] Build passes with new files: `include/color.h`, `src/color.cpp`

---

### US-005: Implement rich diagnostic output
**Description:** As a user, I want to see error messages with surrounding source code and visual indicators pointing to the exact problem location.

**Acceptance Criteria:**
- [ ] `DiagnosticEngine::printAll()` outputs all collected diagnostics
- [ ] `printDiagnostic()` formats single diagnostic with:
  - Header: `filename:line:col: severity: [code] message`
  - Severity text colored (red=error, yellow=warning, cyan=info, green=hint)
  - Source context: 1-2 lines before and after error location
  - Line numbers left-aligned and colored (cyan)
  - Caret (^) or tildes (~~~) under error range in red
- [ ] Related messages printed as `note:` with their own context
- [ ] Suggestions printed as `help:` in green
- [ ] `printSummary()` shows `N errors, M warnings generated`
- [ ] Output format matches modern compilers (Rust, Clang)

---

### US-006: Migrate Lexer to DiagnosticEngine
**Description:** As a developer, I need the lexer to report errors through DiagnosticEngine instead of throwing ParseError.

**Acceptance Criteria:**
- [ ] `Lexer` constructor accepts `DiagnosticEngine&` parameter
- [ ] Lexer errors call `diagnostics.reportError()` instead of throwing
- [ ] Error codes: L001 (unexpected character), L002 (unterminated string), L003 (invalid number), L004 (invalid escape)
- [ ] Lexer continues tokenizing after errors when possible
- [ ] Existing lexer tests updated for new error reporting
- [ ] Build passes; lexer unit tests pass

---

### US-007: Implement parser error recovery
**Description:** As a user, I want the parser to continue after syntax errors so I can see all problems at once.

**Acceptance Criteria:**
- [ ] Parser tracks `in_panic_mode` flag to avoid cascading errors
- [ ] `synchronize()` method skips tokens to next safe point (`;`, `}`, keywords like `if`/`while`/`print`)
- [ ] `expectToken()` method reports error and triggers panic mode instead of throwing
- [ ] `parseProgram()` catches exceptions, synchronizes, and continues parsing
- [ ] Parser produces partial AST even with errors (nullptr for failed nodes)
- [ ] Synchronization prevents reporting 100s of cascading errors
- [ ] Build passes; error recovery tests demonstrate continued parsing

---

### US-008: Migrate Parser to DiagnosticEngine
**Description:** As a developer, I need the parser to report errors through DiagnosticEngine with error recovery.

**Acceptance Criteria:**
- [ ] `Parser` constructor accepts `DiagnosticEngine&` parameter
- [ ] All `throw ParseError(...)` replaced with `diagnostics.reportError(...)` + recovery
- [ ] Error codes: P001 (unexpected token), P002 (expected semicolon), P003 (expected identifier), P004 (expected expression), P005 (unbalanced parens), P006 (unbalanced braces), P007 (invalid assignment target)
- [ ] Parser continues to next statement after errors
- [ ] Existing parser tests updated for multi-error format
- [ ] Build passes; parser unit tests pass

---

### US-009: Migrate TypeChecker to DiagnosticEngine
**Description:** As a developer, I need type checking to report all type errors, not just the first one.

**Acceptance Criteria:**
- [ ] `TypeChecker` constructor accepts `DiagnosticEngine&` parameter
- [ ] Type errors call `diagnostics.reportError()` instead of throwing
- [ ] Error codes: T001 (undefined variable), T002 (type mismatch), T003 (cannot assign to immutable), T004 (variable already declared), T005 (invalid operation)
- [ ] Type checker continues checking after errors
- [ ] "Did you mean?" suggestions for undefined variables using Levenshtein distance
- [ ] Related messages show variable declaration locations
- [ ] Build passes; type checker tests pass

---

### US-010: Implement SuggestionEngine with Levenshtein distance
**Description:** As a user, I want helpful "did you mean?" suggestions when I make typos in variable names.

**Acceptance Criteria:**
- [ ] `SuggestionEngine` class with static helper methods
- [ ] `levenshteinDistance(s1, s2)` calculates edit distance
- [ ] `findSimilarNames(name, available_names, max=3)` returns closest matches (distance ≤ 3)
- [ ] Returns suggestions sorted by distance (closest first)
- [ ] Unit tests verify distance calculation accuracy
- [ ] Unit tests verify suggestion ranking
- [ ] Build passes with implementation in `diagnostic.cpp` or `src/suggestion.cpp`

---

### US-011: Implement common mistake detection
**Description:** As a user, I want the compiler to detect common mistakes and suggest fixes automatically.

**Acceptance Criteria:**
- [ ] `SuggestionEngine::detectCommonMistake()` analyzes error context
- [ ] Detects `=` vs `==` confusion in `if` conditions
- [ ] Detects missing semicolons
- [ ] Detects missing `mut` keyword for reassignment
- [ ] Detects unbalanced parentheses/braces
- [ ] Returns optional suggestion string
- [ ] Suggestions integrated into diagnostic output
- [ ] Unit tests cover each mistake pattern

---

### US-012: Add command-line flags for error control
**Description:** As a user, I want to control error output with command-line flags.

**Acceptance Criteria:**
- [ ] `--no-color` flag disables ANSI color codes
- [ ] `--color` flag forces color even if not TTY
- [ ] `--max-errors N` sets maximum error count (default 10)
- [ ] `--stop-on-first-error` flag for legacy single-error mode
- [ ] `-Werror` flag treats warnings as errors
- [ ] `-Wall` flag enables all warnings
- [ ] Flags parsed in `src/main.cpp`
- [ ] Help text (`--help`) documents all flags
- [ ] Build passes; manual testing confirms flag behavior

---

### US-013: Integrate DiagnosticEngine into main compilation pipeline
**Description:** As a developer, I need the main compiler driver to initialize and use the diagnostic system.

**Acceptance Criteria:**
- [ ] `main()` creates `SourceManager` and loads input file
- [ ] `main()` creates `DiagnosticEngine` with source manager and max_errors
- [ ] Lexer, Parser, TypeChecker initialized with `diagnostics` reference
- [ ] After compilation, `diagnostics.printAll()` outputs all errors/warnings
- [ ] `diagnostics.printSummary()` shows error/warning counts
- [ ] Exit code 1 if `diagnostics.hasErrors()`, else 0
- [ ] Code generation only runs if no errors
- [ ] Build passes; integration tests work end-to-end

---

### US-014: Write unit tests for diagnostic system
**Description:** As a developer, I need comprehensive unit tests for all diagnostic components.

**Acceptance Criteria:**
- [ ] `tests/test_diagnostic.cpp`: DiagnosticEngine collection, severity levels, max error limit
- [ ] `tests/test_source_manager.cpp`: File loading, line access, missing files
- [ ] `tests/test_color.cpp`: Color code generation, TTY detection, enable/disable
- [ ] `tests/test_suggestion.cpp`: Levenshtein distance, similar name finding, common mistakes
- [ ] `tests/test_error_recovery.cpp`: Parser synchronization, continued parsing after errors
- [ ] All tests use Google Test framework
- [ ] Tests pass via `cd build && ctest`
- [ ] `CMakeLists.txt` updated with new test executables

---

### US-015: Create integration tests for multi-error scenarios
**Description:** As a user, I want the compiler to successfully report multiple errors in a single source file.

**Acceptance Criteria:**
- [ ] `tests/k/errors/` directory created
- [ ] `multi_error_undefined.k`: 3 undefined variables → 3 errors reported
- [ ] `multi_error_types.k`: multiple type mismatches → all reported
- [ ] `recovery_semicolon.k`: missing semicolons with recovery → errors + successful later statements
- [ ] `recovery_expression.k`: invalid expressions with recovery
- [ ] Test files include `EXPECTED_ERROR_COUNT: N` comments
- [ ] `test_runner.sh` extended to check error counts (or separate error test runner)
- [ ] All tests pass

---

### US-016: Update existing tests for new error format
**Description:** As a developer, I need existing negative tests to work with the new multi-error diagnostic format.

**Acceptance Criteria:**
- [ ] All tests in `tests/negative/` reviewed
- [ ] Tests expecting specific error messages updated for new format
- [ ] Tests expecting line/column numbers still pass
- [ ] Tests updated to expect error codes (e.g., `[T001]`)
- [ ] Tests expecting single error updated to allow multi-error output
- [ ] All 91/91 negative tests still pass
- [ ] All 37/37 positive integration tests still pass

---

### US-017: Implement related message support for diagnostics
**Description:** As a user, I want to see related locations when an error references another part of the code.

**Acceptance Criteria:**
- [ ] `Diagnostic::addRelatedMessage(msg, range)` stores related diagnostic
- [ ] Undefined variable error shows similar variable declarations as notes
- [ ] Immutable assignment error shows variable declaration as note
- [ ] Type mismatch shows expected type location as note
- [ ] Related messages printed with `note:` prefix and their own source context
- [ ] Multiple related messages supported per diagnostic
- [ ] Integration tests demonstrate related messages

---

### US-018: Implement warning system for code quality
**Description:** As a user, I want warnings about potential issues that don't prevent compilation.

**Acceptance Criteria:**
- [ ] `DiagnosticEngine::reportWarning()` adds warning-level diagnostic
- [ ] Warning codes: W001 (unused variable), W002 (unreachable code), W003 (shadowed variable)
- [ ] Warnings don't prevent code generation
- [ ] Warning count shown in summary
- [ ] `-Werror` flag makes warnings fatal
- [ ] `-Wno-unused` and similar flags to disable specific warnings
- [ ] Warnings colored yellow in output
- [ ] Integration test demonstrates warnings

---

### US-019: Document error codes and diagnostic system
**Description:** As a user, I want documentation explaining error codes and how to interpret diagnostic output.

**Acceptance Criteria:**
- [ ] `docs/error-codes.md` created with all error codes documented
- [ ] Each code has: description, example code, explanation, suggested fix
- [ ] `docs/diagnostic-system.md` explains error output format
- [ ] User guide section on reading error messages
- [ ] Developer guide for adding new diagnostics
- [ ] `memory-bank/activeContext.md` updated with diagnostic system status
- [ ] README.md mentions improved error reporting

---

### US-020: Prepare LSP-compatible diagnostic structure (Optional)
**Description:** As a developer, I want diagnostic data structured for future Language Server Protocol integration.

**Acceptance Criteria:**
- [ ] `Diagnostic::toJSON()` method serializes to JSON
- [ ] JSON format compatible with LSP Diagnostic interface
- [ ] Range format uses zero-based line/column (LSP standard)
- [ ] `--output-format=json` flag outputs diagnostics as JSON array
- [ ] JSON includes: severity, code, message, range, relatedInformation, codeAction hints
- [ ] `DiagnosticEngine::toJSON()` serializes all diagnostics
- [ ] Manual testing with JSON output shows correct structure
- [ ] Documentation notes LSP compatibility

---

## Functional Requirements

- **FR-1:** The diagnostic system must collect up to `max_errors` errors before stopping (default 10).
- **FR-2:** Parser must implement synchronization to recover from syntax errors and continue parsing.
- **FR-3:** Error output must show source context: line numbers, source lines, and visual markers (^/~).
- **FR-4:** Color output must be automatically enabled for TTY and disabled for pipes/CI environments.
- **FR-5:** `--no-color` flag must disable all color output.
- **FR-6:** Diagnostic messages must include error codes (L001, P001, T001, etc.).
- **FR-7:** Type checker must suggest similar names for undefined variables (Levenshtein distance ≤ 3).
- **FR-8:** Common mistakes (=vs==, missing semicolon, missing mut) must generate automatic fix suggestions.
- **FR-9:** Related locations must be shown with `note:` prefix when relevant.
- **FR-10:** Warning system must not prevent code generation unless `-Werror` is specified.
- **FR-11:** Exit code must be 1 if any errors, 0 otherwise.
- **FR-12:** All existing 37/37 positive and 91/91 negative tests must continue to pass after migration.
- **FR-13:** Diagnostic output format must be consistent with modern compilers (Rust, Clang style).
- **FR-14:** Source line context must show 1-2 lines before and after the error location.

---

## Non-Goals (Out of Scope)

- Full Language Server Protocol implementation (only data structure preparation)
- Interactive error fixing (apply suggestions automatically)
- Error message localization (English only)
- Error statistics and analytics
- Custom error formatters beyond plain text and JSON
- Integration with external IDEs (LSP structure prepares for future work)
- Machine learning-based error prediction
- Performance profiling of error-heavy code
- Batch error suppression (individual warning suppression only)

---

## Technical Considerations

- **New files:** 
  - Core: `include/diagnostic.h`, `src/diagnostic.cpp`, `include/source_manager.h`, `src/source_manager.cpp`, `include/color.h`, `src/color.cpp`
  - Optional: `include/suggestion.h`, `src/suggestion.cpp`, `include/error_recovery.h`, `src/error_recovery.cpp`
  - Tests: `tests/test_diagnostic.cpp`, `tests/test_source_manager.cpp`, `tests/test_color.cpp`, `tests/test_suggestion.cpp`, `tests/test_error_recovery.cpp`
  - Integration: `tests/k/errors/multi_error_*.k`, `tests/k/errors/recovery_*.k`
  - Docs: `docs/error-codes.md`, `docs/diagnostic-system.md`

- **Modified files:** 
  - `include/error.h`, `include/lexer.h`, `src/lexer.cpp`, `include/parser.h`, `src/parser.cpp`, `include/type_check.h`, `src/type_check.cpp`, `src/main.cpp`, `CMakeLists.txt`
  - All test files in `tests/negative/` may need updates

- **Dependencies:**
  - C++17 features: `std::optional`, `std::string_view`
  - POSIX: `isatty()` for TTY detection (Unix/Linux/macOS)
  - Standard library: `<map>`, `<vector>`, `<algorithm>`, `<sstream>`

- **Memory considerations:**
  - `SourceManager` caches entire file contents (acceptable for typical small programs)
  - `DiagnosticEngine` stores all diagnostics (bounded by `max_errors`)
  - No significant memory overhead beyond error collection

- **Performance:**
  - Error reporting overhead should be < 10% of total compilation time
  - Levenshtein distance calculation O(n*m) acceptable for small symbol tables
  - File loading happens once per file, cached for all diagnostics

- **Backward compatibility:**
  - Existing error format changes will break tests that match exact error strings
  - Strategy: Update tests incrementally, phase by phase
  - Consider `--legacy-errors` flag if needed during transition

- **Implementation order:** 
  - Phase 1 (Week 1): Core structures (diagnostic.h, source_manager.h, color.h)
  - Phase 2 (Week 1-2): Parser integration and error recovery
  - Phase 3 (Week 2): Rich output and color
  - Phase 4 (Week 2-3): Smart suggestions (SuggestionEngine)
  - Phase 5 (Week 3): Testing and documentation
  - Phase 6 (Optional, Week 3): LSP preparation

---

## Success Metrics

- [ ] All 37/37 existing integration tests continue to pass
- [ ] All 91/91 existing negative tests pass (after format updates)
- [ ] All new unit tests pass (5+ new test suites)
- [ ] Multi-error test demonstrates 3+ errors reported in single run
- [ ] Error recovery test shows parser continues after syntax error
- [ ] Color output correctly detected in TTY and disabled in CI
- [ ] "Did you mean?" suggestion works for undefined variables (80%+ accuracy)
- [ ] Common mistake detection provides helpful fix for at least 3 patterns
- [ ] Error codes documented for all L0xx, P0xx, T0xx, F0xx codes
- [ ] Compilation time increase < 10% for error-heavy code
- [ ] User feedback: subjective improvement in error message clarity and usefulness

---

## Open Questions

1. **Error limit strategy:** Should `max_errors` count only errors, or errors + warnings combined?
   - **Recommendation:** Count only errors; warnings should not be limited unless excessive.

2. **JSON output format:** Should JSON be the default output for machine-readable mode, or only with explicit flag?
   - **Recommendation:** Require explicit `--output-format=json` flag for JSON; keep human-readable as default.

3. **Related message limit:** Should we limit the number of related messages per diagnostic to avoid clutter?
   - **Recommendation:** Limit to 3 related messages per diagnostic; more indicates design issue.

4. **Suggestion confidence:** Should suggestions with low confidence (Levenshtein distance = 3) be shown?
   - **Recommendation:** Show up to 3 suggestions, but only if distance ≤ 3; better to show nothing than wrong suggestions.

5. **Warning defaults:** Should warnings be enabled by default, or require `-Wall`?
   - **Recommendation:** Enable basic warnings (unused variables) by default; advanced warnings require `-Wall`.

6. **Test migration strategy:** Update all 91 negative tests at once, or phase by phase?
   - **Recommendation:** Phase by phase - update as each component migrates to DiagnosticEngine.

7. **Error recovery aggressiveness:** How much should parser guess to recover from errors?
   - **Recommendation:** Conservative recovery - only skip to safe sync points (`;`, `}`), no AST guessing.

8. **LSP compatibility priority:** Should Phase 6 (LSP prep) be part of this project or deferred?
   - **Recommendation:** Include basic JSON serialization; full LSP server is separate future project.

---

## Risk Assessment

### High Risk
- **Test breakage:** All negative tests may break with format changes.
  - *Mitigation:* Update tests incrementally, one phase at a time; consider temporary compatibility mode.

- **Parser complexity:** Error recovery adds significant parser complexity.
  - *Mitigation:* Start with simple synchronization; test extensively; refer to existing compiler implementations.

### Medium Risk
- **Performance regression:** Error collection and formatting may slow compilation.
  - *Mitigation:* Profile after implementation; optimize source context caching; lazy-load file contents.

- **Suggestion accuracy:** Levenshtein distance may suggest wrong names.
  - *Mitigation:* Limit suggestions to distance ≤ 3; rank by distance; show max 3 suggestions.

### Low Risk
- **Color output compatibility:** ANSI codes may not work on all terminals.
  - *Mitigation:* Auto-detect TTY; respect `NO_COLOR` env var; provide `--no-color` flag.

- **Memory usage:** Caching source files may use significant memory for large files.
  - *Mitigation:* ArithLang targets small programs; acceptable tradeoff for better errors.

---

## Timeline Estimate

- **Week 1:** Core structures + Lexer/Parser integration (US-001 to US-008)
- **Week 2:** Rich output + TypeChecker migration (US-009 to US-013)
- **Week 3:** Smart suggestions + Testing + Docs (US-014 to US-019)
- **Optional:** LSP prep (US-020) if time permits

**Total: 2-3 weeks** depending on test migration effort and LSP inclusion.

---

## Acceptance Criteria Summary

**This feature is complete when:**

1. ✅ All 20 user stories have their acceptance criteria met
2. ✅ Build passes: `cmake --build build`
3. ✅ Unit tests pass: `cd build && ctest` (all diagnostic tests green)
4. ✅ Integration tests pass: `./test_runner.sh` (37/37 existing + new error tests)
5. ✅ Multi-error demonstration: At least one test shows 3+ errors in single run
6. ✅ Error recovery demonstration: At least one test shows continued parsing after error
7. ✅ Color output works: Manual test in terminal shows colors; `--no-color` disables
8. ✅ Suggestions work: "Did you mean?" appears for at least one undefined variable test
9. ✅ Documentation complete: Error codes documented, diagnostic system explained
10. ✅ Memory-bank updated: `activeContext.md` reflects diagnostic system status

---

## Implementation Status

**Status:** 🔴 **NOT STARTED**

*To be updated as implementation progresses.*

**Next Steps:**
1. Create US-001: Define diagnostic data structures in `include/diagnostic.h`
2. Create US-002: Implement `DiagnosticEngine` in `src/diagnostic.cpp`
3. Create US-003: Implement `SourceManager` in `src/source_manager.cpp`

---

*Document Created: 2026-03-05*  
*Based on: `plans/error-reporting-improvement-plan.md`*  
*ArithLang Version: v1.5 (Function system complete)*
