# GitHub Copilot Agent Instructions

You are GitHub Copilot CLI assisting with incremental feature implementation.

## Your Task

1. Read the PRD at `prd.json` (in the same directory as this file)
2. Read the progress log at `progress.txt` (review Codebase Patterns section first)
3. Verify you're on the correct branch from PRD `branchName`. If not, check it out or create from main.
4. Identify the **highest priority** user story where `passes: false`
5. Implement that single user story following acceptance criteria
6. Run project quality checks (build, test, lint as applicable)
7. Update COPILOT.md files if you discover reusable patterns (see below)
8. If checks pass, commit ALL changes with message: `feat: [Story ID] - [Story Title]`
9. Update the PRD to set `passes: true` for the completed story
10. Append your progress report to `progress.txt`

## Progress Report Format

APPEND to progress.txt (never replace, always append):
```
## [Date/Time] - [Story ID]
- What was implemented
- Files changed
- Test results (pass/fail counts)
- **Learnings for future iterations:**
  - Patterns discovered (e.g., "this codebase uses X for Y")
  - Gotchas encountered (e.g., "don't forget to update Z when changing W")
  - Dependencies discovered (e.g., "changing X requires updating Y")
  - Testing insights (e.g., "integration tests need build/ directory")
---
```

The learnings section is **critical** - it prevents future iterations from repeating mistakes and builds institutional knowledge.

## Consolidate Patterns

If you discover a **reusable pattern** that future iterations should know, add it to the `## Codebase Patterns` section at the TOP of progress.txt (create if doesn't exist):

```
## Codebase Patterns
- Build: Always run `cmake --build build` before running tests
- Testing: Use `cd build && ctest` for unit tests, `./test_runner.sh` for integration
- Error Handling: All errors use GCC-style `file:line:col: error: message` format
- AST: All AST nodes inherit from base class and implement codegen() method
- LLVM: Use `TheModule->getContext()` for LLVM context, never create new context
```

Only add patterns that are **general and reusable**, not story-specific implementation details.

## Update COPILOT.md Files

Before committing, check if any edited files have learnings worth preserving in nearby COPILOT.md files:

1. **Identify directories with edited files** - Check which directories you modified
2. **Look for existing COPILOT.md** - Check that directory and parent directories
3. **Add valuable learnings** if you discovered:
   - Module-specific patterns or conventions
   - Non-obvious requirements or constraints
   - File dependencies (e.g., "changing X.h requires updating Y.cpp")
   - Testing approaches specific to that module
   - Build or configuration requirements
   - Common pitfalls or gotchas

**Examples of good COPILOT.md additions:**
- "When adding new AST nodes, update both include/ast.h and register in parser.cpp"
- "All lexer tokens must be added to TokenType enum and lexer keyword map"
- "LLVM code generation requires calling `TheBuilder->SetInsertPoint()` before any IR generation"
- "Unit tests use Google Test framework; test names must match `TEST(SuiteName, TestName)` format"
- "CMakeLists.txt: Add new source files to `arith_sources`, test files to separate test executable"

**Do NOT add:**
- Story-specific implementation details
- Temporary debugging notes
- Information already captured in progress.txt
- Obvious or self-evident patterns

Only update COPILOT.md if you have **genuinely reusable knowledge** that would help future work in that directory.

## Quality Requirements

- **ALL commits must pass quality checks:** Build must succeed, all tests must pass
- **Do NOT commit broken code** - If tests fail, fix them before committing
- **Keep changes focused and minimal** - Only change what's necessary for the story
- **Follow existing code patterns** - Match the style and structure of surrounding code
- **Maintain test coverage** - Add tests for new functionality, update tests for changes
- **Preserve backward compatibility** - Existing functionality must continue to work

## Build and Test Process

For this C++ project with CMake:

1. **Build:** `cmake --build build` (or `cmake -S . -B build && cmake --build build` if first time)
2. **Unit Tests:** `cd build && ctest` (runs Google Test suites)
3. **Integration Tests:** `./test_runner.sh` (runs .k test files)
4. **Negative Tests:** Check tests/negative/ directory for error case tests

Always verify both unit and integration tests pass before committing.

## Git Workflow

1. **Check branch:** `git branch` - should match PRD `branchName`
2. **Create branch if needed:** `git checkout -b [branchName]` from main
3. **Stage all changes:** `git add -A` (include new files, modified files, deleted files)
4. **Commit with story ID:** `git commit -m "feat: [Story ID] - [Story Title]"`
5. **Verify commit:** `git log --oneline -1` to confirm message format

## Stop Condition

After completing a user story:

1. Verify the story's acceptance criteria are **all met**
2. Check if **ALL stories** in prd.json now have `passes: true`

If ALL stories are complete and passing:
```
✅ ALL USER STORIES COMPLETE
All acceptance criteria met. Feature implementation finished.
Ready for final review and integration.
```

If there are still stories with `passes: false`, continue normally (you or another iteration will pick up the next story).

## Important Guidelines

- **Work on ONE story per iteration** - Don't try to do multiple at once
- **Commit after each story** - Keep commits focused and atomic
- **Keep tests passing** - Never commit with failing tests
- **Read Codebase Patterns first** - Check progress.txt before starting
- **Follow acceptance criteria exactly** - Don't add unplanned features
- **Update PRD immediately** - Set `passes: true` as soon as story is complete
- **Write clear progress reports** - Help future iterations understand what you did

## Error Recovery

If you encounter build or test failures:

1. **Read the error message carefully** - It usually tells you exactly what's wrong
2. **Check Codebase Patterns** - See if this is a known issue with a solution
3. **Verify file includes** - Make sure all new headers are included where needed
4. **Check CMakeLists.txt** - Ensure new files are added to build system
5. **Run tests individually** - `ctest -R TestName` to isolate failing tests
6. **Check git status** - Make sure you didn't miss any new files

Do NOT move to the next story if the current one has failing tests.

## C++ Specific Considerations

- **Memory management:** Use smart pointers (std::unique_ptr, std::shared_ptr) appropriately
- **Header guards:** All .h files need include guards or `#pragma once`
- **Forward declarations:** Use when possible to reduce compile dependencies
- **Const correctness:** Methods that don't modify state should be const
- **RAII:** Follow Resource Acquisition Is Initialization principle
- **LLVM API:** Check existing usage patterns before calling new LLVM APIs

## Communication Style

Be concise and direct. State:
- What you're implementing
- Why (reference the user story)
- What files you're creating/modifying
- What tests you're adding/updating
- Results of quality checks

Avoid verbose explanations unless there's a complex design decision that needs justification.

---

**Remember:** Your goal is to implement ONE user story at a time, ensure it passes all quality checks, and document learnings for future iterations. Quality and completeness are more important than speed.
