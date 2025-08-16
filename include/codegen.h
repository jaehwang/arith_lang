#pragma once
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "lexer.h"
#include <map>
#include <memory>
#include <vector>
#include <string>

class CodeGen {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
        // Enhanced symbol representation with mutability & scope tracking
        struct Symbol {
                std::string name;
            llvm::AllocaInst* allocaInst;
                bool is_mutable;
                bool is_initialized;
                SourceLocation declaration_site;
                int scope_level;

                Symbol() = default;
            Symbol(const std::string& n,
                   llvm::AllocaInst* a,
                             bool mut = false,
                             bool init = true,
                             SourceLocation loc = {},
                             int scope = 0)
                : name(n), allocaInst(a), is_mutable(mut), is_initialized(init),
                            declaration_site(loc), scope_level(scope) {}
        };

        // Stack of scopes (innermost at back)
        std::vector<std::map<std::string, Symbol>> scopes;
    std::string sourceFileName;
    
public:
    CodeGen(const std::string& moduleName, const std::string& sourceFile = "");
    
    llvm::LLVMContext& getContext() { return *context; }
    llvm::Module& getModule() { return *module; }
    llvm::IRBuilder<>& getBuilder() { return *builder; }

    // Scope management
    void enterScope();
    void exitScope();

    // Variable APIs (enhanced)
    // Back-compat: createVariable declares an immutable variable in the current scope
    llvm::AllocaInst* createVariable(const std::string& name);
    // Declare variable with explicit mutability and location
    llvm::AllocaInst* declareVariable(const std::string& name, bool is_mutable,
                                      const SourceLocation& loc = SourceLocation{});
    // Lookup variable alloca from innermost scope outward
    llvm::AllocaInst* getVariable(const std::string& name);
    // Back-compat setter: sets/overwrites symbol in current scope as immutable
    void setVariable(const std::string& name, llvm::AllocaInst* alloca);

    // Symbol/introspection helpers
    bool canReassign(const std::string& name) const; // true if found and mutable
    bool canShadow(const std::string& name) const;   // always true (language allows)
    // Lookup helpers
    const Symbol* lookupNearestSymbol(const std::string& name) const;
    const Symbol* lookupCurrentSymbol(const std::string& name) const;

    // Convenience helpers for current scope to avoid exposing Symbol outside
    bool hasCurrentSymbol(const std::string& name) const;
    bool isCurrentSymbolMutable(const std::string& name) const;
    llvm::AllocaInst* getCurrentAlloca(const std::string& name) const;
    // Nearest-scope convenience helpers
    bool hasNearestSymbol(const std::string& name) const { return lookupNearestSymbol(name) != nullptr; }
    bool isNearestSymbolMutable(const std::string& name) const {
        auto* s = lookupNearestSymbol(name); return s ? s->is_mutable : false;
    }
    llvm::AllocaInst* getNearestAlloca(const std::string& name) const {
        auto* s = lookupNearestSymbol(name); return s ? s->allocaInst : nullptr;
    }
    
    llvm::Function* getPrintfDeclaration();
    
    void printModule();
    void writeObjectFile(const std::string& filename);
    void setSourceFileName(const std::string& filename);
};