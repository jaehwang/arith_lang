#include "module_resolver.h"
#include "parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

static std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string ModuleResolver::resolveModulePath(const std::string& moduleName, const std::string& currentFile) {
    // Basic resolution: assume moduleName is relative to currentFile's directory
    // or relative to current working directory if currentFile is empty.
    // If moduleName has no extension, append .k
    std::string filename = moduleName;
    if (filename.length() < 2 || filename.substr(filename.length() - 2) != ".k") {
        filename += ".k";
    }

    if (currentFile.empty()) {
        return filename;
    }

    fs::path currPath(currentFile);
    fs::path dir = currPath.parent_path();
    fs::path targetPath = dir / filename;
    return targetPath.string();
}

void ModuleResolver::loadModule(const std::string& moduleName, const std::string& filepath, const SourceLocation& importLoc) {
    if (modules.find(filepath) != modules.end()) {
        // Already loaded or in progress
        if (visiting.count(filepath)) {
            throw ParseError("circular dependency detected: " + filepath, importLoc);
        }
        return;
    }

    visiting.insert(filepath);

    std::string source;
    try {
        source = readFile(filepath);
    } catch (const std::exception& e) {
        throw ParseError("module '" + moduleName + "' not found", importLoc);
    }

    Lexer lexer(source, filepath);
    Parser parser(lexer);
    auto ast = parser.parseProgram();

    auto mod = std::make_unique<ResolvedModule>();
    mod->name = moduleName;
    mod->filepath = filepath;

    for (const auto& imp : ast->getImports()) {
        std::string depPath = resolveModulePath(imp->getModuleName(), filepath);
        mod->dependencies.push_back(depPath);
        loadModule(imp->getModuleName(), depPath, imp->getLocation());
    }

    mod->ast = std::move(ast);
    visiting.erase(filepath);
    modules[filepath] = std::move(mod);
    loadOrder.push_back(filepath);
}

std::unique_ptr<ProgramAST> ModuleResolver::resolve(const std::string& entryFile) {
    SourceLocation entryLoc{entryFile, 1, 1};
    loadModule("main", entryFile, entryLoc);

    std::vector<std::unique_ptr<ImportStmtAST>> combinedImports;
    std::vector<std::unique_ptr<ExportStmtAST>> combinedExports;
    std::vector<std::unique_ptr<ASTNode>> combinedStatements;

    for (const auto& filepath : loadOrder) {
        auto& mod = modules[filepath];
        
        auto imps = mod->ast->releaseImports();
        for (auto& imp : imps) {
            for (const auto& sym : imp->getSymbols()) {
                if (!sym.alias.empty()) {
                    auto varExpr = std::make_unique<VariableExprAST>(sym.name, imp->getLocation());
                    auto assignExpr = std::make_unique<AssignmentExprAST>(
                        sym.alias, std::move(varExpr),
                        false, AssignmentType::DECLARATION, imp->getLocation()
                    );
                    combinedStatements.push_back(std::move(assignExpr));
                }
            }
            combinedImports.push_back(std::move(imp));
        }
        
        auto exps = mod->ast->releaseExports();
        for (auto& exp : exps) {
            combinedExports.push_back(std::move(exp));
        }
        
        auto stmts = mod->ast->releaseStatements();
        for (auto& stmt : stmts) {
            combinedStatements.push_back(std::move(stmt));
        }
    }

    return std::make_unique<ProgramAST>(std::move(combinedImports), std::move(combinedExports), std::move(combinedStatements));
}
