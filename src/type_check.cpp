#include "type_check.h"
#include "function_ast.h"
#include "parser.h" // for ParseError and SourceLocation
#include <stdexcept>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
enum class ValueType { Number, String, Function };

// Combined type info returned from inferExprType (type + optional arity for functions)
struct TypeInfo {
    ValueType type = ValueType::Number;
    int param_count = -1;  // only meaningful when type == Function
};

struct SymbolInfo {
    bool is_mutable = false;
    bool is_parameter = false;  // true for function parameters
    // Where this symbol was first declared/bound (for diagnostics)
    SourceLocation declLoc{};
    ValueType type = ValueType::Number;
    int param_count = -1;  // only meaningful when type == Function
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
    void declare(const std::string& name, bool is_mutable, const SourceLocation& loc,
                 ValueType ty = ValueType::Number, int paramCount = -1, bool isParam = false) {
        if (scopes.empty()) scopes.emplace_back();
        SymbolInfo info;
        info.is_mutable = is_mutable;
        info.is_parameter = isParam;
        info.declLoc = loc;
        info.type = ty;
        info.param_count = paramCount;
        scopes.back()[name] = info;
    }

private:
    std::vector<std::map<std::string, SymbolInfo>> scopes;
};

const char* toTypeName(ValueType t) {
    if (t == ValueType::Number) return "number";
    if (t == ValueType::String) return "string";
    return "function";
}

void typeCheckNode(ASTNode* node, TypeEnv& env, const std::string& filename);

// Infer expression type and validate subexpressions
TypeInfo inferExprType(ExprAST* expr, TypeEnv& env, const std::string& filename) {
    if (!expr) return TypeInfo{ValueType::Number};

    if (auto num = dynamic_cast<NumberExprAST*>(expr)) {
        (void)num;
        return TypeInfo{ValueType::Number};
    }

    if (auto var = dynamic_cast<VariableExprAST*>(expr)) {
        auto* info = env.lookup(var->getName());
        if (!info) {
            throw ParseError("cannot find value '" + var->getName() + "' in this scope", var->getNameLocation());
        }
        return TypeInfo{info->type, info->param_count};
    }

    if (auto str = dynamic_cast<StringLiteralAST*>(expr)) {
        (void)str;
        return TypeInfo{ValueType::String};
    }

    if (auto unary = dynamic_cast<UnaryExprAST*>(expr)) {
        auto t = inferExprType(unary->getOperand(), env, filename);
        if (t.type == ValueType::String) {
            throw ParseError("String literal cannot be used in unary operation", unary->getOperatorLocation());
        }
        return TypeInfo{ValueType::Number};
    }

    if (auto bin = dynamic_cast<BinaryExprAST*>(expr)) {
        auto lt = inferExprType(bin->getLHS(), env, filename);
        auto rt = inferExprType(bin->getRHS(), env, filename);
        if (lt.type == ValueType::String) {
            throw ParseError("String literal cannot be used in binary operation (left operand)", bin->getOperatorLocation());
        }
        if (rt.type == ValueType::String) {
            throw ParseError("String literal cannot be used in binary operation (right operand)", bin->getOperatorLocation());
        }
        return TypeInfo{ValueType::Number};
    }

    if (auto fnLit = dynamic_cast<FunctionLiteralAST*>(expr)) {
        // Check for duplicate parameter names
        std::set<std::string> seenParams;
        for (const auto& p : fnLit->getParams()) {
            if (!seenParams.insert(p.name).second) {
                throw ParseError("duplicate parameter name '" + p.name + "'", p.location);
            }
        }

        // Validate captures: each must exist in outer scope and be mutable
        for (const auto& cap : fnLit->getCaptures()) {
            auto* info = env.lookup(cap.name);
            if (!info) {
                throw ParseError(
                    "captured variable '" + cap.name + "' is not declared in the enclosing scope",
                    cap.location);
            }
            if (!info->is_mutable) {
                throw ParseError(
                    "captured variable '" + cap.name + "' must be declared mutable; consider 'mut " + cap.name + "'",
                    cap.location);
            }
        }

        // Type-check body in function scope with params declared
        env.enterScope();
        for (const auto& p : fnLit->getParams()) {
            env.declare(p.name, p.is_mutable, p.location, ValueType::Number, -1, /*isParam=*/true);
        }
        typeCheckNode(fnLit->getBody(), env, filename);
        env.exitScope();

        return TypeInfo{ValueType::Function, static_cast<int>(fnLit->getParams().size())};
    }

    if (auto call = dynamic_cast<FunctionCallAST*>(expr)) {
        // Infer callee type and check arity if known
        TypeInfo calleeInfo = inferExprType(call->getCallee(), env, filename);
        if (calleeInfo.type == ValueType::Function && calleeInfo.param_count >= 0) {
            int expected = calleeInfo.param_count;
            int actual = static_cast<int>(call->getArgs().size());
            if (actual != expected) {
                throw ParseError(
                    "function expects " + std::to_string(expected) +
                    " argument(s) but " + std::to_string(actual) + " were provided",
                    call->getCallLocation());
            }
        }
        // Type-check arguments
        for (const auto& arg : call->getArgs()) {
            inferExprType(arg.get(), env, filename);
        }
        return TypeInfo{ValueType::Number};
    }

    return TypeInfo{ValueType::Number};
}

void typeCheckExpr(ExprAST* expr, TypeEnv& env, const std::string& filename) {
    (void)inferExprType(expr, env, filename);
}

void typeCheckNode(ASTNode* node, TypeEnv& env, const std::string& filename) {
    if (!node) return;

    // Expression-only nodes
    if (auto expr = dynamic_cast<ExprAST*>(node)) {
        // Handle assignments specially
        if (auto assign = dynamic_cast<AssignmentExprAST*>(expr)) {
            const std::string& name = assign->getVarName();
            bool isMutDecl = assign->isMutableDeclaration();

            // Detect self-referential function literal (recursive function):
            // if RHS is a fn literal that references the LHS name in its body,
            // pre-declare name as Function in the current scope so the body can see it.
            if (!isMutDecl && !env.lookupCurrent(name)) {
                if (auto* fnLit = dynamic_cast<FunctionLiteralAST*>(assign->getValue())) {
                    if (functionBodyReferencesVar(fnLit, name)) {
                        int pc = static_cast<int>(fnLit->getParams().size());
                        env.declare(name, /*is_mutable=*/false, assign->getNameLocation(),
                                    ValueType::Function, pc);
                        inferExprType(assign->getValue(), env, filename);
                        return;  // declaration already in place; skip normal flow
                    }
                }
            }

            // First infer RHS expression type (validates subexpressions)
            TypeInfo rhsInfo = inferExprType(assign->getValue(), env, filename);

            if (isMutDecl) {
                // Explicit mutable declaration always declares in current scope
                env.declare(name, /*is_mutable=*/true, assign->getNameLocation(),
                            rhsInfo.type, rhsInfo.param_count);
            } else {
                // No 'mut' keyword: check existing bindings
                if (auto* cur = env.lookupCurrent(name)) {
                    if (cur->is_mutable) {
                        // Mutation allowed, but type must match
                        if (cur->type != rhsInfo.type) {
                            std::string msg = "mismatched types";
                            if (!cur->declLoc.file.empty()) {
                                msg += std::string("\n") +
                                       "note: expected due to first assignment: " + cur->declLoc.file + ":" +
                                       std::to_string(cur->declLoc.line) + ":" + std::to_string(cur->declLoc.column);
                            }
                            msg += std::string("\n") + "help: expected " + toTypeName(cur->type) + ", found " + toTypeName(rhsInfo.type);
                            throw ParseError(msg, assign->getNameLocation());
                        }
                        cur->param_count = rhsInfo.param_count;
                    } else {
                        // Reassignment to an immutable variable in the same scope
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
                            // Cross-scope mutation allowed — enforce type consistency
                            if (nearest->type != rhsInfo.type) {
                                std::string msg = "mismatched types";
                                if (!nearest->declLoc.file.empty()) {
                                    msg += std::string("\n") +
                                           "note: expected due to first assignment: " + nearest->declLoc.file + ":" +
                                           std::to_string(nearest->declLoc.line) + ":" + std::to_string(nearest->declLoc.column);
                                }
                                msg += std::string("\n") + "help: expected " + toTypeName(nearest->type) + ", found " + toTypeName(rhsInfo.type);
                                throw ParseError(msg, assign->getNameLocation());
                            }
                            nearest->param_count = rhsInfo.param_count;
                        } else if (nearest->is_parameter) {
                            // Reassigning an immutable parameter is an error
                            const auto& firstLoc = nearest->declLoc;
                            std::string msg = "Cannot reassign to immutable parameter '" + name + "'";
                            if (!firstLoc.file.empty()) {
                                msg += std::string("\n") +
                                       "note: parameter declared here: " + firstLoc.file + ":" +
                                       std::to_string(firstLoc.line) + ":" + std::to_string(firstLoc.column);
                            }
                            msg += std::string("\n") + "help: consider declaring the parameter as mutable: 'mut " + name + "'";
                            throw ParseError(msg, assign->getNameLocation());
                        } else {
                            // Shadow with new immutable declaration in current scope
                            env.declare(name, /*is_mutable=*/false, assign->getNameLocation(),
                                        rhsInfo.type, rhsInfo.param_count);
                        }
                    } else {
                        // New immutable declaration in current scope
                        env.declare(name, /*is_mutable=*/false, assign->getNameLocation(),
                                    rhsInfo.type, rhsInfo.param_count);
                    }
                }
            }
            return;
        }

        // All other expressions
        typeCheckExpr(expr, env, filename);
        return;
    }

    // Print statement
    if (auto print = dynamic_cast<PrintStmtAST*>(node)) {
        typeCheckExpr(print->getFormatExpr(), env, filename);
        for (const auto& arg : print->getArgs()) {
            typeCheckExpr(arg.get(), env, filename);
        }
        return;
    }

    // If statement (blocks handle scoping)
    if (auto ifs = dynamic_cast<IfStmtAST*>(node)) {
        typeCheckExpr(ifs->getCondition(), env, filename);
        // then
        env.enterScope();
        typeCheckNode(ifs->getThenStmt(), env, filename);
        env.exitScope();
        // else
        env.enterScope();
        typeCheckNode(ifs->getElseStmt(), env, filename);
        env.exitScope();
        return;
    }

    // While statement
    if (auto wh = dynamic_cast<WhileStmtAST*>(node)) {
        typeCheckExpr(wh->getCondition(), env, filename);
        env.enterScope();
        typeCheckNode(wh->getBody(), env, filename);
        env.exitScope();
        return;
    }

    // Return statement: validate the return expression if present
    if (auto ret = dynamic_cast<ReturnStmtAST*>(node)) {
        if (ret->hasValue()) {
            typeCheckExpr(ret->getValue(), env, filename);
        }
        return;
    }

    // Block: own scope
    if (auto block = dynamic_cast<BlockAST*>(node)) {
        env.enterScope();
        for (const auto& stmt : block->getStatements()) {
            typeCheckNode(stmt.get(), env, filename);
        }
        env.exitScope();
        return;
    }

    // Program: global scope
    if (auto program = dynamic_cast<ProgramAST*>(node)) {
        env.enterScope();
        for (const auto& stmt : program->getStatements()) {
            typeCheckNode(stmt.get(), env, filename);
        }
        env.exitScope();
        return;
    }
}
} // namespace

void typeCheck(ASTNode* node, const std::string& filename) {
    TypeEnv env;
    typeCheckNode(node, env, filename);
}
