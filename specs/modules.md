# ArithLang Module System Specification

## Overview

ArithLang provides a modern module system that enables code organization, reusability, and namespace management. The module system integrates seamlessly with the immutable variable system and function/closure features, supporting both named exports/imports and namespace-based access patterns.

## Core Principles

1. **File-Based Modules**: Each `.k` file is a module
2. **Explicit Exports**: Only explicitly exported items are accessible from other modules
3. **Flexible Imports**: Support for named imports, namespace imports, and aliasing
4. **Immutability Integration**: Exported bindings respect immutability rules
5. **Static Resolution**: Module dependencies resolved at compile time
6. **Circular Dependency Detection**: Prevent circular module dependencies

## Syntax

### Export Statements

```bnf
<export_stmt>    ::= "export" <export_spec> ";"

<export_spec>    ::= <assignment>
                   | <function_def>
                   | "{" <export_list> "}"
                   | "default" <expression>

<export_list>    ::= <export_item> ("," <export_item>)*
<export_item>    ::= <identifier> ("as" <identifier>)?

<function_def>   ::= "fn" <identifier> "(" <parameter_list>? ")" <function_body>
```

### Import Statements

```bnf
<import_stmt>    ::= "import" <import_spec> "from" <string_literal> ";"

<import_spec>    ::= "{" <import_list> "}"
                   | "*" "as" <identifier>
                   | <identifier>

<import_list>    ::= <import_item> ("," <import_item>)*
<import_item>    ::= <identifier> ("as" <identifier>)?
```

## Export System

### 1. Named Exports

```k
// math.k - Math utility module
export PI = 3.14159265359;
export E = 2.71828182846;

export square = fn(x) => x * x;
export cube = fn(x) => x * x * x;

// Multiple exports in one statement
add = fn(x, y) => x + y;
subtract = fn(x, y) => x - y;
multiply = fn(x, y) => x * y;
divide = fn(x, y) => x / y;

export { add, subtract, multiply, divide };

// Export with renaming
internal_helper = fn(x) => x * 2;
export { internal_helper as double };
```

### 2. Function Exports

```k
// utils.k - Utility functions module
export fn is_even(n) {
    return (n % 2) == 0;
}

export fn is_odd(n) {
    return (n % 2) == 1;
}

export fn clamp(value, min, max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
```

### 3. Default Exports

```k
// logger.k - Default export example
log_level = "info";

log = fn(message) {
    print "[" + log_level + "] " + message;
};

set_level = fn(level) {
    log_level = level;
};

// Default export (single primary export)
export default {
    log: log,
    set_level: set_level
};
```

### 4. Mutable Exports

```k
// state.k - Shared mutable state
export mut global_counter = 0;
export mut application_config = {
    debug: false,
    port: 8080
};

export increment_counter = fn() mut(global_counter) {
    global_counter = global_counter + 1;
    return global_counter;
};
```

## Import System

### 1. Named Imports

```k
// main.k - Using named imports
import { PI, square, cube } from "math";
import { is_even, clamp } from "utils";

fn main() {
    radius = 5.0;
    area = PI * square(radius);
    volume = (4.0 / 3.0) * PI * cube(radius);
    
    print "Area:", area;
    print "Volume:", volume;
    print "Is 4 even?", is_even(4);
    print "Clamped 15:", clamp(15, 0, 10);
}
```

### 2. Namespace Imports

```k
// main.k - Using namespace imports
import * as math from "math";
import * as utils from "utils";

fn main() {
    result = math.square(math.PI);
    clamped = utils.clamp(result, 0, 100);
    print "Result:", clamped;
}
```

### 3. Import Aliasing

```k
// main.k - Using aliases to avoid conflicts
import { add as math_add, multiply } from "math";
import { add as string_add } from "strings";

fn main() {
    num_result = math_add(5, 3);
    str_result = string_add("Hello", " World");
    print num_result;
    print str_result;
}
```

### 4. Default Imports

```k
// main.k - Using default imports
import logger from "logger";
import { log as direct_log } from "logger";  // named import from same module

fn main() {
    logger.set_level("debug");
    logger.log("Application started");
    direct_log("Direct function call");
}
```

### 5. Mixed Imports

```k
// main.k - Combining different import styles
import default_logger, { PI, square } from "math";
import * as utils from "utils";
import { mut global_counter, increment_counter } from "state";

fn main() {
    area = PI * square(5);
    even = utils.is_even(global_counter);
    new_count = increment_counter();
    
    print "Area:", area;
    print "Counter was even:", even;
    print "New counter:", new_count;
}
```

## Module Resolution

### 1. File System Mapping

```
project/
├── main.k              # Entry point
├── math.k              # "math" module
├── utils/              
│   ├── strings.k       # "utils/strings" module
│   └── arrays.k        # "utils/arrays" module
└── lib/
    └── external.k      # "lib/external" module
```

### 2. Module Path Resolution

```k
// Relative imports (from current directory)
import { helper } from "utils";        // -> utils.k
import { join } from "utils/strings";  // -> utils/strings.k

// Absolute imports (from project root)
import { config } from "/config";      // -> config.k (from root)

// Library imports (from lib directory)  
import { api } from "lib/external";    // -> lib/external.k
```

### 3. Module Loading Order

1. **Parse Phase**: Parse all referenced modules
2. **Dependency Analysis**: Build dependency graph
3. **Circular Check**: Detect and report circular dependencies
4. **Topological Sort**: Order modules for initialization
5. **Compilation**: Generate code for each module
6. **Linking**: Resolve inter-module references

## Namespace Management

### 1. Module Scope Isolation

```k
// math.k
internal_constant = 42;  // Private to module
export public_constant = 100;  // Accessible from other modules

helper = fn(x) => x * internal_constant;  // Private function
export compute = fn(x) => helper(x) + public_constant;  // Public function
```

### 2. Name Collision Resolution

```k
// main.k - Handling name collisions
import { add } from "math";        // math.add
import { add } from "strings";     // ERROR: name collision

// Solution 1: Use aliases
import { add as math_add } from "math";
import { add as string_add } from "strings";

// Solution 2: Use namespace imports
import * as math from "math";
import * as strings from "strings";
// Access as math.add and strings.add
```

### 3. Re-exports

```k
// index.k - Module aggregation and re-exports
import { PI, E } from "constants";
import { square, cube } from "geometry";
import { sin, cos } from "trigonometry";

// Re-export everything as a unified API
export { PI, E };
export { square, cube };
export { sin, cos };

// Computed re-exports
export TAU = 2 * PI;
export circle_area = fn(r) => PI * square(r);
```

## Integration with Language Features

### 1. Immutable Exports

```k
// config.k
export VERSION = "1.0.0";
export MAX_USERS = 1000;

// main.k
import { VERSION, MAX_USERS } from "config";
// VERSION = "2.0.0";  // ERROR: cannot reassign immutable import
```

### 2. Mutable Exports

```k
// state.k
export mut current_user = null;
export mut session_count = 0;

// main.k  
import { mut current_user, mut session_count } from "state";
current_user = "Alice";      // OK: mutable import
session_count = 1;           // OK: mutable import
```

### 3. Function and Closure Exports

```k
// factory.k
export make_counter = fn(initial) {
    mut count = initial;
    return fn() mut(count) {
        count = count + 1;
        return count;
    };
};

export make_validator = fn(min, max) => fn(value) => {
    return value >= min && value <= max;
};

// main.k
import { make_counter, make_validator } from "factory";

fn main() {
    counter = make_counter(10);
    validator = make_validator(0, 100);
    
    print counter();        // 11
    print validator(50);    // true
    print validator(150);   // false
}
```

### 4. Higher-Order Function Exports

```k
// functional.k
export map = fn(array, transform) {
    mut result = [];
    for (item in array) {
        result.push(transform(item));
    }
    return result;
};

export filter = fn(array, predicate) {
    mut result = [];
    for (item in array) {
        if (predicate(item)) {
            result.push(item);
        }
    }
    return result;
};

export compose = fn(f, g) => fn(x) => f(g(x));

// main.k
import { map, filter, compose } from "functional";
import { square } from "math";
import { is_even } from "utils";

fn main() {
    numbers = [1, 2, 3, 4, 5, 6];
    
    // Use imported higher-order functions
    squares = map(numbers, square);
    evens = filter(numbers, is_even);
    
    // Compose functions
    square_if_even = compose(
        fn(x) => is_even(x) ? square(x) : 0,
        fn(x) => x
    );
    
    print "Squares:", squares;
    print "Evens:", evens;
}
```

## Module System Compilation Model

### 1. Compilation Units

```cpp
// Compiler representation
struct Module {
    std::string name;
    std::string file_path;
    std::vector<std::string> dependencies;
    std::map<std::string, ExportedSymbol> exports;
    std::vector<ImportDeclaration> imports;
    std::unique_ptr<ProgramAST> ast;
};

struct ExportedSymbol {
    std::string name;
    std::string internal_name;  // for aliasing
    SymbolType type;
    bool is_mutable;
    bool is_default;
};

struct ImportDeclaration {
    std::string module_name;
    ImportType type;  // Named, Namespace, Default
    std::vector<ImportedSymbol> symbols;
};
```

### 2. Dependency Resolution Algorithm

```cpp
class ModuleResolver {
private:
    std::map<std::string, Module> modules;
    std::vector<std::string> resolution_order;
    
public:
    void loadModule(const std::string& module_path);
    void resolveDependencies();
    void detectCircularDependencies();
    std::vector<std::string> getCompilationOrder();
};

// Topological sort for module ordering
std::vector<std::string> ModuleResolver::getCompilationOrder() {
    // Kahn's algorithm implementation
    std::map<std::string, int> in_degree;
    std::queue<std::string> queue;
    std::vector<std::string> result;
    
    // Calculate in-degrees and perform topological sort
    // ...
    
    return result;
}
```

### 3. Symbol Resolution

```cpp
class SymbolResolver {
private:
    std::map<std::string, Module*> loaded_modules;
    
public:
    Symbol* resolveImport(const std::string& module_name, 
                         const std::string& symbol_name);
    void validateExport(Module* module, const std::string& symbol_name);
    void checkNameCollisions(const std::vector<ImportDeclaration>& imports);
};
```

## Error Handling

### 1. Module Not Found

```k
import { missing_func } from "nonexistent";
```

**Error Message:**
```
error: module 'nonexistent' not found
  --> main.k:1:30
   |
1  | import { missing_func } from "nonexistent";
   |                              ^^^^^^^^^^^^^ module not found
   |
note: checked paths:
  - ./nonexistent.k
  - ./nonexistent/index.k
  - ./lib/nonexistent.k
```

### 2. Export Not Found

```k
import { undefined_function } from "math";
```

**Error Message:**
```
error: 'undefined_function' is not exported by module 'math'
  --> main.k:1:10
   |
1  | import { undefined_function } from "math";
   |          ^^^^^^^^^^^^^^^^^^ not found in module exports
   |
note: available exports from 'math': PI, E, square, cube, add, subtract
```

### 3. Circular Dependencies

```k
// a.k
import { b_func } from "b";
export a_func = fn() => b_func();

// b.k  
import { a_func } from "a";
export b_func = fn() => a_func();
```

**Error Message:**
```
error: circular dependency detected
  --> a.k:1:1
   |
1  | import { b_func } from "b";
   |  ^^^^^^^^^^^^^^^^^^^^^^^^^^
   |
note: dependency cycle: a -> b -> a
```

### 4. Name Collision

```k
import { add } from "math";
import { add } from "strings";
```

**Error Message:**
```
error: name 'add' imported multiple times
  --> main.k:2:10
   |
1  | import { add } from "math";
   |          --- first import here
2  | import { add } from "strings";
   |          ^^^ name collision
   |
help: use aliases to resolve the conflict:
  import { add as math_add } from "math";
  import { add as string_add } from "strings";
```

### 5. Immutable Import Reassignment

```k
import { PI } from "math";
PI = 3.14;  // ERROR
```

**Error Message:**
```
error: cannot assign to immutable import 'PI'
  --> main.k:2:1
   |
2  | PI = 3.14;
   |  ^^ cannot modify imported binding
   |
help: if you need to modify this value, import it as mutable:
  import { mut PI } from "math";
```

## Build System Integration

### 1. Project Structure

```
project/
├── arithc.toml         # Project configuration
├── src/
│   ├── main.k          # Entry point
│   ├── math.k          # Math module
│   └── utils/
│       ├── strings.k
│       └── arrays.k
├── lib/                # External libraries
└── build/              # Compiled output
    ├── main.ll
    ├── math.ll
    └── utils/
        ├── strings.ll
        └── arrays.ll
```

### 2. Project Configuration (arithc.toml)

```toml
[project]
name = "my_app"
version = "1.0.0"
entry_point = "src/main.k"

[build]
output_dir = "build"
target = "llvm-ir"
optimization_level = 2

[dependencies]
math_lib = { path = "lib/math" }
utils = { version = "1.2.0" }
```

### 3. Compilation Commands

```bash
# Compile entire project
arithc build

# Compile specific module
arithc build src/math.k

# Run with dependency resolution
arithc run src/main.k

# Check for errors without compilation
arithc check
```

## Implementation Strategy

### 1. Parser Extensions

```cpp
class ModuleParser : public Parser {
private:
    std::vector<ImportDeclaration> imports;
    std::vector<ExportDeclaration> exports;
    
public:
    std::unique_ptr<ImportDeclaration> parseImportStatement();
    std::unique_ptr<ExportDeclaration> parseExportStatement();
    
    // Enhanced program parsing with module support
    std::unique_ptr<ModuleAST> parseModule();
};

class ModuleAST : public ASTNode {
private:
    std::vector<std::unique_ptr<ImportDeclaration>> imports;
    std::vector<std::unique_ptr<ExportDeclaration>> exports;
    std::vector<std::unique_ptr<ASTNode>> statements;
    
public:
    const std::vector<ImportDeclaration*> getImports() const;
    const std::vector<ExportDeclaration*> getExports() const;
    Value* codegen() override;
};
```

### 2. Module Compilation Pipeline

```cpp
class ModuleCompiler {
private:
    ModuleResolver resolver;
    SymbolResolver symbol_resolver;
    std::map<std::string, llvm::Module*> compiled_modules;
    
public:
    void compileProject(const std::string& entry_point);
    void compileModule(const std::string& module_path);
    void linkModules();
    void generateExecutable(const std::string& output_path);
    
private:
    void validateModuleDependencies();
    void generateModuleIR(Module* module);
    void resolveModuleImports(Module* module);
};
```

### 3. Runtime Module System

```cpp
// Module registry for runtime symbol resolution
class ModuleRegistry {
private:
    std::map<std::string, ModuleInstance> loaded_modules;
    
public:
    struct ModuleInstance {
        std::map<std::string, Value*> exported_symbols;
        llvm::Module* llvm_module;
        bool is_initialized;
    };
    
    void registerModule(const std::string& name, ModuleInstance instance);
    Value* resolveSymbol(const std::string& module, const std::string& symbol);
    void initializeModule(const std::string& name);
};
```

## Test Cases

### 1. Basic Export/Import

```k
// TEST: math.k
export PI = 3.14159;
export square = fn(x) => x * x;

// TEST: main.k
// EXPECTED: 78.539750000000000
import { PI, square } from "math";
fn main() {
    radius = 5.0;
    area = PI * square(radius);
    print area;
}
```

### 2. Namespace Import

```k
// TEST: utils.k
export double = fn(x) => x * 2;
export triple = fn(x) => x * 3;

// TEST: main.k
// EXPECTED: 10.000000000000000
// EXPECTED: 15.000000000000000
import * as utils from "utils";
fn main() {
    print utils.double(5);
    print utils.triple(5);
}
```

### 3. Mutable Export/Import

```k
// TEST: counter.k
export mut global_count = 0;
export increment = fn() mut(global_count) {
    global_count = global_count + 1;
    return global_count;
};

// TEST: main.k
// EXPECTED: 1.000000000000000
// EXPECTED: 2.000000000000000
// EXPECTED: 2.000000000000000
import { mut global_count, increment } from "counter";
fn main() {
    print increment();      // 1
    print increment();      // 2
    print global_count;     // 2
}
```

### 4. Re-exports

```k
// TEST: math_basic.k
export add = fn(x, y) => x + y;
export multiply = fn(x, y) => x * y;

// TEST: math_extended.k
import { add, multiply } from "math_basic";
export { add, multiply };  // re-export
export power = fn(x, y) => x ** y;

// TEST: main.k
// EXPECTED: 15.000000000000000
// EXPECTED: 50.000000000000000
import { add, multiply } from "math_extended";
fn main() {
    print add(10, 5);
    print multiply(10, 5);
}
```

### 5. Higher-Order Function Modules

```k
// TEST: functional.k
export map = fn(array, transform) {
    // simplified implementation for test
    return [transform(array[0]), transform(array[1])];
};

// TEST: main.k
// EXPECTED: [4.000000000000000, 9.000000000000000]
import { map } from "functional";
fn main() {
    square = fn(x) => x * x;
    result = map([2, 3], square);
    print result;
}
```

## Future Extensions

### 1. Package Management

```k
// Package imports (future)
import { http_client } from "@arithlib/network";
import { json_parse } from "@stdlib/json";
```

### 2. Conditional Compilation

```k
// Platform-specific imports (future)
#[cfg(target = "web")]
import { web_api } from "web";

#[cfg(target = "native")]  
import { file_system } from "fs";
```

### 3. Module Versioning

```k
// Version-specific imports (future)
import { api_v2 } from "service@2.0.0";
import { legacy_api } from "service@1.0.0";
```

This specification provides a comprehensive module system that integrates seamlessly with ArithLang's immutable variables and function/closure systems, enabling scalable code organization and reusability.