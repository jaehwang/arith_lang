# AI Agent Instructions

This file contains instructions that AI must strictly follow. AI must adhere to the instructions in this file and those referenced within it.

## Memory Bank Instructions

AI must read and follow the memory bank instructions in rules/memory-bank.md file at the start of every session.

## Session Semantics

To avoid ambiguity, a "session" is defined and managed as follows:

- What is a session: A continuous AI working context within this repository workspace.

- Session starts when any of the following occurs:
	- This repository/workspace is opened in the editor for the first time in a run.
	- The first AI interaction for this repo in a new chat/conversation thread.
	- Git branch is switched or new changes are pulled that may affect policies.
	- The editor window is reloaded or the agent/tools are restarted.

- Actions on session start (MANDATORY):
	1) Read and apply `rules/memory-bank.md`.
	2) Check for directory-specific `AGENTS.md` when reading or modifying files in directories that contain them.
	3) Resolve policy precedence as: directory `AGENTS.md` > root `AGENTS.md` > `rules/*` guidance.

- Session ends when:
	- The workspace is closed or a different repository/workspace is opened.

## Code Comments

AI must follow the comment guidelines in rules/anchor-comments.md file.

## Commit Guidelines

AI must follow the commit message writing guidelines in rules/commit-log.md file.

## Test Commands

- **Build**: `cmake --build build`
- **Unit tests**: `cd build && ctest` 
- **Integration tests**: `./test_runner.sh` (MUST be run from project root directory)

## Directory-Specific AGENTS.md Files

* **Always check for `AGENTS.md` files in specific directories** when reading or modifying files within them. These files contain targeted context.
* Policy precedence: directory `AGENTS.md` > root `AGENTS.md` > `rules/*` guidance.
