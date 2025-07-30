# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Memory Bank Instructions

Claude must read and follow the memory bank instructions in @.github/instructions/memory-bank.instructions.md at the start of every session. This defines how Claude should work with the memory bank system.

The memory bank files in `memory-bank/` directory contain current project context:
- @memory-bank/projectbrief.md: Core requirements and scope
- @memory-bank/activeContext.md: Current work focus and recent changes  
- @memory-bank/progress.md: Completed components and current status
- @memory-bank/systemPatterns.md: Architecture and design patterns
- @memory-bank/techContext.md: Technologies and development setup
- @memory-bank/productContext.md: Project purpose and user experience goals
