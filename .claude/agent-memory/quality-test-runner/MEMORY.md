# Quality Test Runner Memory — ArithLang

## Project Overview
ArithLang is an educational compiler (Lexer → Parser → Type Checker → LLVM Codegen) with full first-class function/closure support.

## Test Commands (from project root)
- Build: `cmake --build build`
- Unit tests: `cd /home/worker/work/ai_coding/arith_lang/build && ctest --output-on-failure`
- Integration tests: `./test_runner.sh` (MUST be run from `/home/worker/work/ai_coding/arith_lang`)

## Test Suite Inventory (as of 2026-03-09)
### Unit Tests (ctest): 11 suites
1. LexerTests
2. ParserTests
3. ParserNegativeTests (91 negative cases)
4. CodeGenTests
5. IntegrationTests
6. SourceLocationTests
7. ParserErrorHelpersTests
8. ErrorDisplayTests
9. IntegrationErrorReportingTests
10. MutabilityTests
11. FunctionParserTests

### Integration Tests (test_runner.sh): 37 tests
- Includes 6 function_*.k tests (closures, higher-order, recursive, etc.)
- Error-path tests use `// EXPECTED:` annotations in .k files for stderr matching

## Baseline Status (2026-03-09)
- Build: PASS (all targets built cleanly, including test_function_parser)
- Unit tests: 11/11 PASS (0 failures, ~2.05s total)
- Integration tests: 37/37 PASS (0 failures)

## Key Patterns & Notes
- Integration tests validate both stdout (program output) and stderr (compile errors via EXPECTED comments)
- Negative/error integration tests rely on `// EXPECTED:` lines in `.k` source files
- All tests are deterministic and fast (full suite runs in ~2-3s)
- LLVM 21 deprecation warning for `PointerType::getUnqual()` was already fixed
- No flaky tests observed

## Test File Locations
- Unit test sources: `tests/test_*.cpp`
- Integration test .k programs: `tests/k/*.k`
- Test runner script: `./test_runner.sh`
- CMake build dir: `build/`
