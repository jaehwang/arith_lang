---
mode: agent
---
# Deep Planning Prompt

Before implementing complex features, please approach them systematically through thorough codebase investigation, requirement clarification, and detailed planning. Follow this four-step process that mirrors how senior developers approach complex features.

## The Three-Step Process

### Step 1: Silent Investigation

Investigate the codebase like a detective, silently exploring to understand its structure, patterns, and constraints. Examine source files, analyze import patterns, discover class hierarchies, and identify technical debt markers. No commentary, no narration - just focused research.

During this phase, run commands like:
- Finding all class and function definitions across the codebase
- Analyzing import patterns to understand dependencies
- Discovering project structure and file organization
- Identifying TODOs and technical debt

### Step 2: Discussion and Questions

Once you understand the codebase, ask targeted questions that will shape the implementation. These aren't generic questions - they're specific to the project and the feature being built.

Questions might cover:
- Clarifying ambiguous requirements
- Choosing between equally valid implementation approaches
- Confirming assumptions about system behavior
- Understanding preferences for technical decisions

### Step 3: Implementation Plan Document

Create a structured markdown document (`implementation_plan.md`) that serves as an implementation blueprint. This isn't a vague outline - it's a detailed specification with exact file paths, function signatures, and implementation order.

The plan includes eight comprehensive sections:
- **Overview**: The goal and high-level approach
- **Types**: Complete type definitions and data structures
- **Files**: Exact files to create, modify, or delete
- **Functions**: New and modified functions with signatures
- **Classes**: Class modifications and inheritance details
- **Dependencies**: Package requirements and versions
- **Testing**: Validation strategies and test requirements
- **Implementation Order**: Step-by-step execution sequence

## Using Deep Planning

Start a deep planning session by requesting:

```
Please perform deep planning for [feature description]
```

Example:
```
Please perform deep planning for user authentication with JWT tokens and role-based access control
```

The investigation will begin immediately. You'll see files being read and commands executed to understand the codebase. Once enough context is gathered, discussion will begin before creating the plan.

## Example Workflow

Here's how to use deep planning for a real feature:

### Step 1: Initiate Planning
Request: `Please perform deep planning for implementing a caching layer for API responses`

### Step 2: Silent Investigation
Explore the codebase, examining:
- Current API structure and endpoints
- Existing data flow patterns
- Database queries and performance bottlenecks
- Configuration and environment setup

### Step 3: Targeted Discussion
Ask questions like:
- "Should we use Redis or in-memory caching?"
- "What's the acceptable cache staleness for user data?"
- "Do you need cache invalidation webhooks?"

### Step 4: Plan Creation
Generate `implementation_plan.md` with:
- Cache service class specifications
- Redis connection configuration
- Modified API endpoints with caching logic
- Cache key generation strategies
- TTL configurations for different data types

## Best Practices

### When to Use Deep Planning

Use Deep Planning for:
- Features touching multiple parts of your codebase
- Architectural changes requiring careful coordination
- Complex integrations with external services
- Refactoring efforts that need systematic execution
- Any feature where you'd normally spend time whiteboarding

### Making the Most of Investigation

Let the investigation complete thoroughly. The quality of the plan directly correlates with how well the codebase is understood. If you have specific areas that should be examined, mention them in your initial request.

### Reviewing the Plan

Always review `implementation_plan.md` before starting implementation. The plan is comprehensive but not immutable - you can edit it directly if needed. Think of it as a collaborative document between you and the AI.

### Tracking Progress

With Focus Chain enabled, your implementation progress displays in the task header. Each completed step gets checked off automatically as work progresses through the plan, giving you real-time visibility into complex implementations.

## Inspiration

Use Deep Planning whenever you're about to build something that would normally require a design document. Recent examples from workflows:

- **Migrating authentication systems**: Deep Planning mapped every endpoint, identified all authentication touchpoints, and created a migration plan that avoided breaking changes.

- **Adding real-time features**: The plan covered WebSocket integration, event handling, state synchronization, and fallback mechanisms for disconnections.

- **Database schema refactoring**: Identified all affected queries, created migration scripts, and planned the rollout to minimize downtime.

- **API versioning implementation**: The plan detailed route changes, backward compatibility layers, deprecation notices, and client migration paths.

The power of Deep Planning is that it forces thoughtful architecture before implementation. It's like having a senior developer review your approach before you write code, except that developer has perfect knowledge of your entire codebase.

**Note**: Deep Planning requires models with strong reasoning capabilities. It works best with the latest generation of models, like GPT-5, Claude 4, Gemini 2.5, or Grok 4. Smaller models may struggle with the comprehensive analysis required.

For simpler tasks that don't require extensive planning, consider creating focused tasks with context, or jump straight into implementation if the path forward is clear.
