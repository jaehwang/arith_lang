#include "type_check.h"
#include "parser.h" // for ParseError and SourceLocation
#include <stdexcept>
#include <map>
#include <string>
#include <vector>

namespace {
enum class ValueType { Number, String };

struct SymbolInfo {
    bool is_mutable = false;
    // Where this symbol was first declared/bound (for diagnostics)
    SourceLocation declLoc{};
    ValueType type = ValueType::Number;
};

class TypeEnv {
public:
    void enterScope() { scopes.emplace_back(); }
    void exitScope() { if (!scopes.empty()) scopes.pop_back(); }

    // Returns ptr to symbol if found in any scope (innermost outward)
    SymbolInfo* lookup(const std::string& name) {
        for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return &it->second;
        }
        return nullptr;
    }

    // Returns ptr to symbol if found in current scope
    SymbolInfo* lookupCurrent(const std::string& name) {
        if (scopes.empty()) return nullptr;
        auto it = scopes.back().find(name);
        if (it != scopes.back().end()) return &it->second;
        return nullptr;
    }

    // Declare/overwrite in current scope (shadowing allowed)
    void declare(const std::string& name, bool is_mutable, const SourceLocation& loc, ValueType ty = ValueType::Number) {
        if (scopes.empty()) scopes.emplace_back();
        SymbolInfo info;
        info.is_mutable = is_mutable;
        info.declLoc = loc;
        info.type = ty;
        scopes.back()[name] = info;
    }

private:
    std::vector<std::map<std::string, SymbolInfo>> scopes;
};

const char* toTypeName(ValueType t) { return t == ValueType::Number ? "number" : "string"; }

void typeCheckNode(ASTNode* node, TypeEnv& env);

// Infer expression type and validate subexpressions
ValueType inferExprType(ExprAST* expr, TypeEnv& env) {
    if (!expr) return ValueType::Number;

    if (auto num = dynamic_cast<NumberExprAST*>(expr)) {
        (void)num;
        return ValueType::Number;
    }

    if (auto var = dynamic_cast<VariableExprAST*>(expr)) {
        auto* info = env.lookup(var->getName());
        if (!info) {
            throw ParseError("cannot find value '" + var->getName() + "' in this scope", var->getNameLocation());
        }
        return info->type;
    }

    if (auto str = dynamic_cast<StringLiteralAST*>(expr)) {
        (void)str;
        return ValueType::String;
    }

    if (auto unary = dynamic_cast<UnaryExprAST*>(expr)) {
        auto t = inferExprType(unary->getOperand(), env);
        if (t == ValueType::String) {
            throw std::runtime_error("String literal cannot be used in unary operation");
        }
        return ValueType::Number;
    }

    if (auto bin = dynamic_cast<BinaryExprAST*>(expr)) {
        auto lt = inferExprType(bin->getLHS(), env);
        auto rt = inferExprType(bin->getRHS(), env);
        if (lt == ValueType::String) {
            throw std::runtime_error("String literal cannot be used in binary operation (left operand)");
        }
        if (rt == ValueType::String) {
            throw std::runtime_error("String literal cannot be used in binary operation (right operand)");
        }
        return ValueType::Number;
    }

    return ValueType::Number;
}

void typeCheckExpr(ExprAST* expr, TypeEnv& env) {
    (void)inferExprType(expr, env);
}

void typeCheckNode(ASTNode* node, TypeEnv& env) {
    if (!node) return;

    // Expression-only nodes
    if (auto expr = dynamic_cast<ExprAST*>(node)) {
        // Handle assignments specially
    if (auto assign = dynamic_cast<AssignmentExprAST*>(expr)) {
            // First infer RHS expression type (validates subexpressions)
            ValueType rhsType = inferExprType(assign->getValue(), env);

            const std::string& name = assign->getVarName();
            bool isMutDecl = assign->isMutableDeclaration();

            if (isMutDecl) {
                // Explicit mutable declaration always declares in current scope
                env.declare(name, /*is_mutable=*/true, assign->getNameLocation(), rhsType);
            } else {
                // No 'mut' keyword: if the current scope has a mutable symbol, treat as mutation.
                // Otherwise, if nearest (outer) symbol is mutable, allow mutation across scopes.
                // Else, treat as shadowing/new immutable declaration (backward compatible).
                if (auto* cur = env.lookupCurrent(name)) {
                    if (cur->is_mutable) {
                        // Mutation allowed, but type must match
                        if (cur->type != rhsType) {
                            std::string msg = "mismatched types";
                            // attach note/help similar to spec
                            if (!cur->declLoc.file.empty()) {
                                msg += std::string("\n") +
                                       "note: expected due to first assignment: " + cur->declLoc.file + ":" +
                                       std::to_string(cur->declLoc.line) + ":" + std::to_string(cur->declLoc.column);
                            }
                            msg += std::string("\n") + "help: expected " + toTypeName(cur->type) + ", found " + toTypeName(rhsType);
                            throw ParseError(msg, assign->getNameLocation());
                        }
                    } else {
                        // Reassignment to an immutable variable in the same scope is not allowed
                        // Enhance message with original declaration site and a help hint
                        const auto& firstLoc = cur->declLoc;
                        std::string msg = "Cannot reassign to immutable variable '" + name + "'";
                        if (!firstLoc.file.empty()) {
                            msg += std::string("\n") +
                                   "note: first assignment here: " + firstLoc.file + ":" +
                                   std::to_string(firstLoc.line) + ":" + std::to_string(firstLoc.column);
                        }
                        msg += std::string("\n") + "help: consider making this binding mutable: 'mut " + name + "'";
                        throw ParseError(msg, assign->getNameLocation());
                    }
                } else {
                    // Not in current scope — check nearest outer binding
                    if (auto* nearest = env.lookup(name)) {
                        if (nearest->is_mutable) {
                            // Cross-scope mutation allowed — no new declaration (enforce type)
                            if (nearest->type != rhsType) {
                                std::string msg = "mismatched types";
                                if (!nearest->declLoc.file.empty()) {
                                    msg += std::string("\n") +
                                           "note: expected due to first assignment: " + nearest->declLoc.file + ":" +
                                           std::to_string(nearest->declLoc.line) + ":" + std::to_string(nearest->declLoc.column);
                                }
                                msg += std::string("\n") + "help: expected " + toTypeName(nearest->type) + ", found " + toTypeName(rhsType);
                                throw ParseError(msg, assign->getNameLocation());
                            }
                        } else {
                            // Shadow with new immutable declaration in current scope
                            env.declare(name, /*is_mutable=*/false, assign->getNameLocation(), rhsType);
                        }
                    } else {
                        // New immutable declaration in current scope
                        env.declare(name, /*is_mutable=*/false, assign->getNameLocation(), rhsType);
                    }
                }
            }
            return;
        }

        // All other expressions
        typeCheckExpr(expr, env);
        return;
    }

    // Print statement
    if (auto print = dynamic_cast<PrintStmtAST*>(node)) {
        typeCheckExpr(print->getFormatExpr(), env);
        for (const auto& arg : print->getArgs()) {
            typeCheckExpr(arg.get(), env);
        }
        return;
    }

    // If statement (blocks handle scoping)
    if (auto ifs = dynamic_cast<IfStmtAST*>(node)) {
        typeCheckExpr(ifs->getCondition(), env);
        // then
        env.enterScope();
        typeCheckNode(ifs->getThenStmt(), env);
        env.exitScope();
        // else
        env.enterScope();
        typeCheckNode(ifs->getElseStmt(), env);
        env.exitScope();
        return;
    }

    // While statement
    if (auto wh = dynamic_cast<WhileStmtAST*>(node)) {
        typeCheckExpr(wh->getCondition(), env);
        env.enterScope();
        typeCheckNode(wh->getBody(), env);
        env.exitScope();
        return;
    }

    // Block: own scope
    if (auto block = dynamic_cast<BlockAST*>(node)) {
        env.enterScope();
        for (const auto& stmt : block->getStatements()) {
            typeCheckNode(stmt.get(), env);
        }
        env.exitScope();
        return;
    }

    // Program: global scope
    if (auto program = dynamic_cast<ProgramAST*>(node)) {
        env.enterScope();
        for (const auto& stmt : program->getStatements()) {
            typeCheckNode(stmt.get(), env);
        }
        env.exitScope();
        return;
    }
}
} // namespace

void typeCheck(ASTNode* node) {
    TypeEnv env;
    typeCheckNode(node, env);
}
