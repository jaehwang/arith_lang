#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

class ModuleResolver {
public:
    struct ResolvedModule {
        std::string name;
        std::string filepath;
        std::unique_ptr<ProgramAST> ast;
        std::vector<std::string> dependencies;
    };

    ModuleResolver() = default;

    // Resolves all dependencies starting from the entry file.
    // Returns a combined ProgramAST with all statements in topological order.
    std::unique_ptr<ProgramAST> resolve(const std::string& entryFile);

private:
    std::map<std::string, std::unique_ptr<ResolvedModule>> modules;
    std::vector<std::string> loadOrder;
    std::set<std::string> visiting;

    void loadModule(const std::string& moduleName, const std::string& filepath, const SourceLocation& importLoc);
    std::string resolveModulePath(const std::string& moduleName, const std::string& currentFile);
};
