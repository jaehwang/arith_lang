# ArithLang Entry Point and Execution Model Specification

## Overview

ArithLang provides a flexible execution model that supports both simple script-style execution and structured program-style execution with explicit entry points. The system automatically detects the execution mode based on the presence of a `main` function and integrates seamlessly with the module system, immutable variables, and function/closure features.

## Core Principles

1. **Dual Execution Models**: Support both script mode and program mode
2. **Automatic Mode Detection**: Compiler automatically detects execution mode
3. **Backward Compatibility**: Existing script-style code continues to work unchanged
4. **Module Integration**: Entry points work with multi-module projects
5. **Progressive Complexity**: Simple scripts can evolve into structured programs
6. **Type Safety**: Entry point validation with argument type checking

## Execution Models

### 1. Script Mode (Backward Compatible)

**Characteristics:**
- No explicit `main` function
- Top-level statements execute sequentially
- Suitable for simple calculations and scripts
- Maintains compatibility with existing ArithLang code

```k
// simple_script.k - Script Mode
x = 42;
y = x * 2;
print "Result:", y;

// This executes immediately when run
```

**Generated LLVM Structure:**
```llvm
; Script mode generates implicit main function
define double @main() {
entry:
  ; Execute all top-level statements
  %x = alloca double
  store double 42.0, %x
  %y = alloca double
  %0 = load double, %x
  %1 = fmul double %0, 2.0
  store double %1, %y
  ; ... print statement code ...
  ret double 0.0
}
```

### 2. Program Mode (Structured)

**Characteristics:**
- Explicit `main` function as entry point
- Clear separation between initialization and execution
- Supports command-line arguments (future)
- Better suited for larger applications

```k
// structured_app.k - Program Mode
// Global constants and function definitions
VERSION = "1.0.0";
calculate = fn(x, y) => x * y + 10;

// Entry point
fn main() {
    print "Application", VERSION, "starting...";
    
    result = calculate(5, 3);
    print "Calculation result:", result;
    
    return 0;  // Optional return code
}
```

**Generated LLVM Structure:**
```llvm
; Program mode generates structured main function
define double @main() {
entry:
  ; Call user-defined main function
  %0 = call double @user_main()
  ret double %0
}

define double @user_main() {
entry:
  ; User's main function implementation
  ; ... application logic ...
  ret double 0.0
}
```

## Entry Point Detection

### 1. Automatic Mode Detection Algorithm

```cpp
enum class ExecutionMode {
    SCRIPT_MODE,    // No main function found
    PROGRAM_MODE    // Main function found
};

class ExecutionModeAnalyzer {
public:
    ExecutionMode detectMode(const ModuleAST* module) {
        // Search for main function declaration
        for (const auto& statement : module->getStatements()) {
            if (auto funcDef = dynamic_cast<FunctionDefAST*>(statement.get())) {
                if (funcDef->getName() == "main") {
                    return ExecutionMode::PROGRAM_MODE;
                }
            }
        }
        return ExecutionMode::SCRIPT_MODE;
    }
};
```

### 2. Main Function Signature Validation

```k
// Valid main function signatures
fn main() {                    // No parameters, no return
    print "Hello, World!";
}

fn main() {                    // No parameters, void return
    print "Application started";
    return;
}

fn main() {                    // No parameters, numeric return (exit code)
    if (initialize_system()) {
        return 0;              // Success
    } else {
        return 1;              // Error
    }
}

// Future: Command line arguments
fn main(args) {                // Arguments array (future feature)
    print "Arguments:", args.length;
    return 0;
}
```

### 3. Multiple Main Function Error

```k
// ERROR: Multiple main functions not allowed
fn main() {
    print "First main";
}

fn main() {                    // ERROR: Duplicate main function
    print "Second main";
}
```

**Error Message:**
```
error: multiple main functions defined
  --> main.k:5:1
   |
1  | fn main() {
   |    ---- first main function defined here
...
5  | fn main() {
   |    ^^^^ duplicate main function
   |
note: only one main function is allowed per module
```

## Multi-Module Entry Points

### 1. Entry Point Resolution in Projects

**Project Structure:**
```
project/
├── main.k              # Primary entry point
├── app/
│   ├── server.k        # Alternative entry point
│   └── client.k        # Alternative entry point
└── lib/
    └── utils.k         # Library module (no main)
```

### 2. Entry Point Specification

```bash
# Default entry point (main.k)
arithc run

# Explicit entry point
arithc run app/server.k

# Entry point with dependencies
arithc run --entry-point app/client.k --deps lib/utils.k
```

### 3. Module with Main Function

```k
// server.k - Standalone module with main
import { Logger } from "lib/logger";
import { Config } from "lib/config";

// Module-level initialization
logger = Logger.create("server");
config = Config.load("server.conf");

// Entry point for this module
fn main() {
    logger.info("Server starting...");
    
    port = config.get("port", 8080);
    host = config.get("host", "localhost");
    
    print "Starting server on", host + ":" + port;
    
    // Server logic here
    return 0;
}
```

### 4. Library Module (No Main)

```k
// utils.k - Library module without main
export VERSION = "1.0.0";

export format_date = fn(timestamp) {
    // Date formatting logic
    return "formatted_date";
};

export validate_email = fn(email) {
    // Email validation logic
    return email.contains("@");
};

// No main function - this is a library module
```

## Integration with Language Features

### 1. Main Function with Immutable Variables

```k
// Global immutable configuration
API_VERSION = "v1";
MAX_RETRIES = 3;

fn main() {
    // These are accessible but cannot be modified
    print "API Version:", API_VERSION;
    print "Max retries:", MAX_RETRIES;
    
    // Local mutable state
    mut retry_count = 0;
    mut success = false;
    
    while (retry_count < MAX_RETRIES && !success) {
        success = attempt_connection();
        if (!success) {
            retry_count = retry_count + 1;
            print "Retry", retry_count, "of", MAX_RETRIES;
        }
    }
    
    return success ? 0 : 1;
}
```

### 2. Main Function with Closures

```k
// Function factories at module level
make_handler = fn(name) => fn(event) {
    print "Handler", name, "processing:", event;
};

fn main() {
    // Create specialized handlers
    auth_handler = make_handler("Auth");
    data_handler = make_handler("Data");
    
    // Simulate event processing
    auth_handler("login_request");
    data_handler("save_data");
    
    return 0;
}
```

### 3. Main Function with Module Imports

```k
// main.k - Entry point with dependencies
import { Logger } from "lib/logger";
import { Database } from "lib/database";
import { WebServer } from "lib/server";
import * as config from "config";

fn main() {
    // Initialize systems
    logger = Logger.create(config.LOG_LEVEL);
    db = Database.connect(config.DATABASE_URL);
    server = WebServer.create(config.SERVER_PORT);
    
    // Application lifecycle
    try {
        logger.info("Application starting...");
        server.start();
        logger.info("Application ready");
        
        // Main application loop
        while (server.is_running()) {
            server.handle_requests();
        }
        
        return 0;
    } catch (error) {
        logger.error("Application error:", error);
        return 1;
    } finally {
        db.disconnect();
        logger.info("Application shutdown complete");
    }
}
```

### 4. Initialization Order with Modules

```k
// Constants and functions are initialized before main
// config.k
export DEBUG = true;
export PORT = 8080;

// logger.k  
import { DEBUG } from "config";
export log = fn(message) {
    if (DEBUG) {
        print "[LOG]", message;
    }
};

// main.k
import { log } from "logger";
import { PORT } from "config";

// Module initialization happens before main is called
init_message = "System initialized";  // This runs before main

fn main() {
    log("Main function started");      // This runs after module init
    log("Server will start on port: " + PORT);
    return 0;
}
```

## Advanced Entry Point Patterns

### 1. Conditional Main Execution

```k
// utils.k - Library that can also run standalone
export add = fn(x, y) => x + y;
export multiply = fn(x, y) => x * y;

// Conditional main for testing/demo purposes
fn main() {
    // Only runs when executed directly, not when imported
    if (is_main_module()) {
        print "Running utils demo...";
        print "2 + 3 =", add(2, 3);
        print "2 * 3 =", multiply(2, 3);
        return 0;
    }
}

// Built-in function to detect if module is entry point
is_main_module = fn() {
    // Implementation provided by runtime
    return __is_main_module__;
};
```

### 2. Main Function with Error Handling

```k
// Comprehensive error handling in main
import { Result, Error } from "lib/result";

initialize_app = fn() {
    // Simulation of initialization that might fail
    if (system_ready()) {
        return Result.ok("Initialization successful");
    } else {
        return Result.error("System not ready");
    }
};

fn main() {
    result = initialize_app();
    
    match result {
        Ok(message) => {
            print message;
            run_application();
            return 0;
        },
        Error(error_msg) => {
            print "Initialization failed:", error_msg;
            return 1;
        }
    }
}
```

### 3. Main Function with Async Pattern (Future)

```k
// Future: Async main function support
async fn main() {
    print "Starting async application...";
    
    // Parallel initialization
    db_task = async { Database.connect() };
    cache_task = async { Cache.connect() };
    
    db = await db_task;
    cache = await cache_task;
    
    print "All systems ready";
    return 0;
}
```

## Compilation and Code Generation

### 1. Script Mode Code Generation

```cpp
class ScriptModeCodeGen {
public:
    void generateMain(const std::vector<ASTNode*>& statements) {
        // Create implicit main function
        FunctionType* mainType = FunctionType::get(
            Type::getDoubleTy(context), false);
        Function* mainFunc = Function::Create(
            mainType, Function::ExternalLinkage, "main", module);
        
        BasicBlock* entryBB = BasicBlock::Create(context, "entry", mainFunc);
        builder.SetInsertPoint(entryBB);
        
        // Generate code for all top-level statements
        for (auto* stmt : statements) {
            stmt->codegen();
        }
        
        // Return 0.0 (success)
        builder.CreateRet(ConstantFP::get(context, APFloat(0.0)));
    }
};
```

### 2. Program Mode Code Generation

```cpp
class ProgramModeCodeGen {
public:
    void generateMain(FunctionDefAST* userMain, 
                     const std::vector<ASTNode*>& globalStatements) {
        // Generate global initialization code
        generateGlobalInitialization(globalStatements);
        
        // Generate user's main function
        Function* userMainFunc = generateUserMain(userMain);
        
        // Generate system main function that calls user main
        generateSystemMain(userMainFunc);
    }
    
private:
    Function* generateUserMain(FunctionDefAST* userMain) {
        // Generate user's main function with mangled name
        return userMain->codegen("user_main");
    }
    
    void generateSystemMain(Function* userMainFunc) {
        // Create system main that calls user main
        FunctionType* mainType = FunctionType::get(
            Type::getDoubleTy(context), false);
        Function* mainFunc = Function::Create(
            mainType, Function::ExternalLinkage, "main", module);
        
        BasicBlock* entryBB = BasicBlock::Create(context, "entry", mainFunc);
        builder.SetInsertPoint(entryBB);
        
        // Call user main and return its result
        Value* result = builder.CreateCall(userMainFunc);
        builder.CreateRet(result);
    }
};
```

### 3. Multi-Module Entry Point Resolution

```cpp
class ProjectCompiler {
public:
    void compileProject(const std::string& entryPoint) {
        // Load all modules
        std::vector<Module*> modules = loadAllModules();
        
        // Find entry point module
        Module* entryModule = findEntryPointModule(entryPoint, modules);
        
        if (!entryModule) {
            throw CompilerError("Entry point not found: " + entryPoint);
        }
        
        // Validate entry point
        validateEntryPoint(entryModule);
        
        // Generate code for all modules
        for (Module* module : getCompilationOrder(modules)) {
            compileModule(module);
        }
        
        // Link modules
        linkModules(modules);
    }
    
private:
    void validateEntryPoint(Module* module) {
        ExecutionMode mode = detectExecutionMode(module);
        
        if (mode == ExecutionMode::PROGRAM_MODE) {
            validateMainFunction(module);
        }
        // Script mode always valid
    }
    
    void validateMainFunction(Module* module) {
        FunctionDefAST* mainFunc = findMainFunction(module);
        
        // Validate signature
        if (mainFunc->getParameterCount() > 1) {
            throw CompilerError("Main function can have at most 1 parameter");
        }
        
        // Validate return type (should be numeric or void)
        // Implementation details...
    }
};
```

## Error Handling and Validation

### 1. Invalid Main Function Signature

```k
fn main(x, y, z) {  // ERROR: Too many parameters
    return 0;
}
```

**Error Message:**
```
error: main function cannot have more than 1 parameter
  --> main.k:1:9
   |
1  | fn main(x, y, z) {
   |         ^^^^^^^ too many parameters
   |
note: main function can have at most 1 parameter for command-line arguments
help: consider: fn main() or fn main(args)
```

### 2. Main Function Not Found in Program Mode

```bash
arithc run app.k  # app.k has no main function and no top-level code
```

**Error Message:**
```
error: no entry point found in 'app.k'
  |
note: add a main function or top-level statements to make this executable
help: add one of:
  - fn main() { ... }  (structured program)
  - top-level statements (script mode)
```

### 3. Multiple Entry Points in Project

```bash
arithc build  # Multiple .k files with main functions
```

**Error Message:**
```
error: multiple entry points found in project
  --> main.k:5:1
   |
5  | fn main() {
   |    ---- entry point found here
   |
  --> server.k:10:1
   |
10| fn main() {
   |    ---- entry point found here
   |
note: specify entry point explicitly: arithc run main.k
```

## Test Cases

### 1. Script Mode Execution

```k
// TEST: script_mode.k
// EXPECTED: 84.000000000000000
x = 42;
y = x * 2;
print y;
```

### 2. Program Mode Execution

```k
// TEST: program_mode.k
// EXPECTED: Hello from main!
// EXPECTED: 42.000000000000000
calculate = fn(x) => x * 2;

fn main() {
    print "Hello from main!";
    result = calculate(21);
    print result;
    return 0;
}
```

### 3. Main with Return Code

```k
// TEST: main_with_return.k
// EXIT_CODE: 42
fn main() {
    print "Returning custom exit code";
    return 42;
}
```

### 4. Module Import in Main

```k
// TEST: utils.k
export double = fn(x) => x * 2;

// TEST: main_with_import.k
// EXPECTED: 20.000000000000000
import { double } from "utils";

fn main() {
    result = double(10);
    print result;
    return 0;
}
```

### 5. Global Initialization Order

```k
// TEST: initialization_order.k
// EXPECTED: Global init: 100
// EXPECTED: Main function: 100
GLOBAL_VALUE = 50 * 2;  // This runs before main
print "Global init:", GLOBAL_VALUE;

fn main() {
    print "Main function:", GLOBAL_VALUE;
    return 0;
}
```

### 6. Conditional Main Execution

```k
// TEST: conditional_main.k
// When run directly: EXPECTED: Running as main module
// When imported: No output from main

export utility_function = fn(x) => x + 1;

fn main() {
    if (is_main_module()) {
        print "Running as main module";
        print "Utility test:", utility_function(41);
    }
    return 0;
}
```

## Runtime Support

### 1. Built-in Functions

```k
// Built-in functions available in main context
is_main_module();     // Returns true if current module is entry point
get_args();           // Returns command line arguments (future)
exit(code);           // Explicit exit with code (future)
```

### 2. Environment Variables (Future)

```k
fn main() {
    debug_mode = get_env("DEBUG", "false");
    if (debug_mode == "true") {
        print "Debug mode enabled";
    }
    return 0;
}
```

### 3. Program Lifecycle Hooks (Future)

```k
// Lifecycle hooks
on_startup(fn() {
    print "Program starting...";
});

on_shutdown(fn() {
    print "Program shutting down...";
});

fn main() {
    // Main application logic
    return 0;
}
```

This specification provides a comprehensive entry point system that seamlessly integrates with all other ArithLang features while maintaining backward compatibility and supporting both simple scripts and complex applications.