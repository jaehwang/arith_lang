#include "codegen.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h" // for ParseError
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include <stdexcept>
#include <vector>

static std::unique_ptr<CodeGen> codeGenInstance;

CodeGen::CodeGen(const std::string& moduleName, const std::string& sourceFile) : sourceFileName(sourceFile) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    // Initialize global scope
    scopes.emplace_back();
    
    if (!sourceFileName.empty()) {
        module->setSourceFileName(sourceFileName);
    }
}

llvm::AllocaInst* CodeGen::createVariable(const std::string& name) {
    return declareVariable(name, /*is_mutable=*/false, SourceLocation{sourceFileName, 1, 1});
}

llvm::AllocaInst* CodeGen::declareVariable(const std::string& name, bool is_mutable,
                                           const SourceLocation& loc) {
    // Ensure we have a valid insertion point and function (unit tests may call without setup)
    llvm::Function* function = nullptr;
    if (auto* insertBB = builder->GetInsertBlock()) {
        function = insertBB->getParent();
    }
    if (!function) {
        // Create a private stub function and entry block for allocations
        auto* funcTy = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), /*isVarArg=*/false);
        function = llvm::Function::Create(
            funcTy,
            llvm::Function::InternalLinkage,
            "__init",
            *module
        );
        auto* entryBB = llvm::BasicBlock::Create(*context, "entry", function);
        builder->SetInsertPoint(entryBB);
    }
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst* alloca = tmpB.CreateAlloca(llvm::Type::getDoubleTy(*context), nullptr, name);
    int scope_level = static_cast<int>(scopes.size()) - 1;
    scopes.back()[name] = Symbol{name, alloca, is_mutable, true, loc, scope_level};
    return alloca;
}

llvm::AllocaInst* CodeGen::getVariable(const std::string& name) {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
    if (it != scopes[i].end()) return it->second.allocaInst;
    }
    return nullptr;
}

void CodeGen::setVariable(const std::string& name, llvm::AllocaInst* alloca) {
    int scope_level = static_cast<int>(scopes.size()) - 1;
    scopes.back()[name] = Symbol{name, alloca, /*is_mutable=*/false, true,
                                 SourceLocation{sourceFileName, 1, 1}, scope_level};
}

void CodeGen::enterScope() {
    scopes.emplace_back();
}

void CodeGen::exitScope() {
    if (!scopes.empty()) scopes.pop_back();
    if (scopes.empty()) {
        // Ensure there is always at least one scope to avoid edge cases
        scopes.emplace_back();
    }
}

bool CodeGen::canReassign(const std::string& name) const {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return it->second.is_mutable;
    }
    return false;
}

bool CodeGen::canShadow(const std::string& /*name*/) const {
    return true; // Language allows shadowing in inner scopes
}

const CodeGen::Symbol* CodeGen::lookupNearestSymbol(const std::string& name) const {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

const CodeGen::Symbol* CodeGen::lookupCurrentSymbol(const std::string& name) const {
    if (scopes.empty()) return nullptr;
    auto it = scopes.back().find(name);
    if (it != scopes.back().end()) return &it->second;
    return nullptr;
}

bool CodeGen::hasCurrentSymbol(const std::string& name) const {
    return lookupCurrentSymbol(name) != nullptr;
}

bool CodeGen::isCurrentSymbolMutable(const std::string& name) const {
    auto* s = lookupCurrentSymbol(name);
    return s ? s->is_mutable : false;
}

llvm::AllocaInst* CodeGen::getCurrentAlloca(const std::string& name) const {
    auto* s = lookupCurrentSymbol(name);
    return s ? s->allocaInst : nullptr;
}

llvm::Function* CodeGen::getPrintfDeclaration() {
    llvm::Function* printfFunc = module->getFunction("printf");
    if (!printfFunc) {
        llvm::FunctionType* printfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*context),
            {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*context))}, 
            true
        );
        printfFunc = llvm::Function::Create(
            printfType, 
            llvm::Function::ExternalLinkage, 
            "printf", 
            *module
        );
    }
    return printfFunc;
}

void CodeGen::printModule() {
    module->print(llvm::outs(), nullptr);
}

void CodeGen::setSourceFileName(const std::string& filename) {
    sourceFileName = filename;
    if (module) {
        module->setSourceFileName(filename);
    }
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(val));
}

llvm::Value* VariableExprAST::codegen() {
    llvm::AllocaInst* alloca = codeGenInstance->getVariable(name);
    if (!alloca) {
    // Propagate as ParseError with identifier location for better diagnostics
    throw ParseError("cannot find value '" + name + "' in this scope", name_location);
    }
    return codeGenInstance->getBuilder().CreateLoad(llvm::Type::getDoubleTy(codeGenInstance->getContext()), alloca, name);
}

llvm::Value* StringLiteralAST::codegen() {
    return codeGenInstance->getBuilder().CreateGlobalString(value, "str");
}

llvm::Value* UnaryExprAST::codegen() {
    llvm::Value* operandV = operand->codegen();
    if (!operandV) return nullptr;
    
    switch (op) {
        case '-':
            return codeGenInstance->getBuilder().CreateFNeg(operandV, "negtmp");
        default:
            throw std::runtime_error("Invalid unary operator");
    }
}

llvm::Value* BinaryExprAST::codegen() {
    llvm::Value* l = lhs->codegen();
    llvm::Value* r = rhs->codegen();
    if (!l || !r) return nullptr;
    
    switch (op) {
        case '+':
            return codeGenInstance->getBuilder().CreateFAdd(l, r, "addtmp");
        case '-':
            return codeGenInstance->getBuilder().CreateFSub(l, r, "subtmp");
        case '*':
            return codeGenInstance->getBuilder().CreateFMul(l, r, "multmp");
        case '/':
            return codeGenInstance->getBuilder().CreateFDiv(l, r, "divtmp");
        case TOK_LT: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOLT(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_LTE: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOLE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_GT: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOGT(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_GTE: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOGE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_EQ: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpOEQ(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        case TOK_NEQ: {
            llvm::Value* cmp = codeGenInstance->getBuilder().CreateFCmpONE(l, r, "cmptmp");
            return codeGenInstance->getBuilder().CreateUIToFP(cmp, llvm::Type::getDoubleTy(codeGenInstance->getContext()), "booltmp");
        }
        default:
            throw std::runtime_error("Invalid binary operator");
    }
}

llvm::Value* AssignmentExprAST::codegen() {
    llvm::Value* val = value->codegen();
    if (!val) return nullptr;

    // Determine how to handle binding based on mutability and scope
    const bool isMutDecl = is_mutable_declaration;
    llvm::AllocaInst* targetAlloca = nullptr;

    if (isMutDecl) {
        // Explicit mutable declaration: always create a new alloca in current scope
        targetAlloca = codeGenInstance->declareVariable(varName, /*is_mutable=*/true,
                        SourceLocation{codeGenInstance->getModule().getSourceFileName(), 1, 1});
    } else {
        // No 'mut'
        if (codeGenInstance->hasCurrentSymbol(varName) && codeGenInstance->isCurrentSymbolMutable(varName)) {
            // Mutation of existing mutable variable in current scope
            targetAlloca = codeGenInstance->getCurrentAlloca(varName);
        } else if (codeGenInstance->hasCurrentSymbol(varName)) {
            // Shadowing: new immutable binding (redeclare in current scope)
            targetAlloca = codeGenInstance->declareVariable(varName, /*is_mutable=*/false,
                            SourceLocation{codeGenInstance->getModule().getSourceFileName(), 1, 1});
        } else {
            // Not present in current scope. If outer mutable exists, mutate it; otherwise, shadow with new immutable.
            if (codeGenInstance->hasNearestSymbol(varName) && codeGenInstance->isNearestSymbolMutable(varName)) {
                targetAlloca = codeGenInstance->getNearestAlloca(varName);
            } else {
                targetAlloca = codeGenInstance->declareVariable(varName, /*is_mutable=*/false,
                                SourceLocation{codeGenInstance->getModule().getSourceFileName(), 1, 1});
            }
        }
    }

    codeGenInstance->getBuilder().CreateStore(val, targetAlloca);
    return val;
}

llvm::Value* PrintStmtAST::codegen() {
    llvm::Function* printfFunc = codeGenInstance->getPrintfDeclaration();
    
    // Generate code for the format expression
    llvm::Value* formatVal = formatExpr->codegen();
    if (!formatVal) return nullptr;
    
    // Check if we have a format string with arguments
    StringLiteralAST* formatString = dynamic_cast<StringLiteralAST*>(formatExpr.get());
    
    if (formatString && !args.empty()) {
        // This is a format string with arguments - parse the format string
        std::string formatStr = formatString->getValue();
        std::vector<llvm::Value*> printfArgs;
        
        // Start with the format string itself
        printfArgs.push_back(formatVal);
        
        // Process each argument
        size_t argIndex = 0;
        std::string processedFormat;
        
        for (size_t i = 0; i < formatStr.length(); i++) {
            if (formatStr[i] == '%' && i + 1 < formatStr.length()) {
                if (formatStr[i + 1] == '%') {
                    // Literal % - just add both characters
                    processedFormat += "%%";
                    i++; // skip next character
                } else if (formatStr[i + 1] == 'f' || 
                          formatStr[i + 1] == 'g' || 
                          formatStr[i + 1] == 'e' || 
                          formatStr[i + 1] == 'd' ||
                          formatStr[i + 1] == 's') {
                    // Simple format specifier
                    if (argIndex < args.size()) {
                        llvm::Value* arg = args[argIndex]->codegen();
                        if (!arg) return nullptr;
                        
                        // Handle type conversions
                        if (formatStr[i + 1] == 'd') {
                            // Convert double to int
                            arg = codeGenInstance->getBuilder().CreateFPToSI(
                                arg, llvm::Type::getInt32Ty(codeGenInstance->getContext()), "dtoi");
                            processedFormat += "%d";
                        } else if (formatStr[i + 1] == 's') {
                            // String argument - check if it's a string literal
                            StringLiteralAST* stringArg = dynamic_cast<StringLiteralAST*>(args[argIndex].get());
                            if (!stringArg) {
                                throw std::runtime_error("%s format specifier requires string literal argument");
                            }
                            processedFormat += "%s";
                        } else {
                            // Floating point formats (f, g, e)
                            processedFormat += '%';
                            processedFormat += formatStr[i + 1];
                        }
                        
                        printfArgs.push_back(arg);
                        argIndex++;
                    } else {
                        throw std::runtime_error("Not enough arguments for format string");
                    }
                    i++; // skip format character
                } else if (formatStr[i + 1] == '.' && i + 2 < formatStr.length() && 
                          std::isdigit(formatStr[i + 2])) {
                    // Handle precision specifiers like %.2f
                    size_t j = i + 2;
                    while (j < formatStr.length() && std::isdigit(formatStr[j])) j++;
                    
                    if (j < formatStr.length() && (formatStr[j] == 'f' || formatStr[j] == 'g' || formatStr[j] == 'e')) {
                        if (argIndex < args.size()) {
                            llvm::Value* arg = args[argIndex]->codegen();
                            if (!arg) return nullptr;
                            
                            // Copy the entire format specifier
                            processedFormat += formatStr.substr(i, j - i + 1);
                            printfArgs.push_back(arg);
                            argIndex++;
                        } else {
                            throw std::runtime_error("Not enough arguments for format string");
                        }
                        i = j; // skip to after format character
                    } else {
                        processedFormat += formatStr[i];
                    }
                } else {
                    processedFormat += formatStr[i];
                }
            } else {
                processedFormat += formatStr[i];
            }
        }
        
        if (argIndex < args.size()) {
            throw std::runtime_error("Too many arguments for format string");
        }
        
        // Create the processed format string and call printf (no automatic newline)
        llvm::Constant* finalFormatStr = codeGenInstance->getBuilder().CreateGlobalString(processedFormat);
        printfArgs[0] = finalFormatStr; // Replace the original format string
        
        return codeGenInstance->getBuilder().CreateCall(printfFunc, printfArgs, "printfcall");
        
    } else if (formatString) {
        // String literal without arguments - process %% and output with %s (no automatic newline)
        std::string str = formatString->getValue();
        
        // Process %% -> % in string literals
        std::string processedStr;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '%' && i + 1 < str.length() && str[i + 1] == '%') {
                processedStr += '%';
                i++; // skip next %
            } else {
                processedStr += str[i];
            }
        }
        
        // Create processed string and use %s format
        llvm::Constant* processedStringVal = codeGenInstance->getBuilder().CreateGlobalString(processedStr, "str");
        llvm::Constant* formatStr = codeGenInstance->getBuilder().CreateGlobalString("%s");
        std::vector<llvm::Value*> printfArgs = {formatStr, processedStringVal};
        return codeGenInstance->getBuilder().CreateCall(printfFunc, printfArgs, "printfcall");
    } else {
        // Numeric expression - use high precision format
        llvm::Constant* formatStr = codeGenInstance->getBuilder().CreateGlobalString("%.15f\n");
        std::vector<llvm::Value*> printfArgs = {formatStr, formatVal};
        return codeGenInstance->getBuilder().CreateCall(printfFunc, printfArgs, "printfcall");
    }
}

llvm::Value* IfStmtAST::codegen() {
    llvm::Value* condV = condition->codegen();
    if (!condV) return nullptr;
    
    // Convert condition to boolean by comparing with 0.0
    condV = codeGenInstance->getBuilder().CreateFCmpONE(
        condV, 
        llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0)), 
        "ifcond"
    );
    
    llvm::Function* function = codeGenInstance->getBuilder().GetInsertBlock()->getParent();
    
    // Create blocks for then, else, and merge
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "ifcont");
    
    codeGenInstance->getBuilder().CreateCondBr(condV, thenBB, elseBB);
    
    // Emit then block
    codeGenInstance->getBuilder().SetInsertPoint(thenBB);
    llvm::Value* thenV = thenStmt->codegen();
    if (!thenV) return nullptr;
    
    // Convert to double if needed
    if (thenV->getType()->isIntegerTy()) {
        thenV = codeGenInstance->getBuilder().CreateSIToFP(
            thenV, 
            llvm::Type::getDoubleTy(codeGenInstance->getContext()), 
            "int2double"
        );
    }
    
    codeGenInstance->getBuilder().CreateBr(mergeBB);
    thenBB = codeGenInstance->getBuilder().GetInsertBlock();
    
    // Emit else block
    function->insert(function->end(), elseBB);
    codeGenInstance->getBuilder().SetInsertPoint(elseBB);
    
    llvm::Value* elseV = nullptr;
    if (elseStmt) {
        elseV = elseStmt->codegen();
        if (!elseV) return nullptr;
        
        // Convert to double if needed
        if (elseV->getType()->isIntegerTy()) {
            elseV = codeGenInstance->getBuilder().CreateSIToFP(
                elseV, 
                llvm::Type::getDoubleTy(codeGenInstance->getContext()), 
                "int2double"
            );
        }
    } else {
        elseV = llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
    }
    
    codeGenInstance->getBuilder().CreateBr(mergeBB);
    elseBB = codeGenInstance->getBuilder().GetInsertBlock();
    
    // Emit merge block
    function->insert(function->end(), mergeBB);
    codeGenInstance->getBuilder().SetInsertPoint(mergeBB);
    
    llvm::PHINode* pn = codeGenInstance->getBuilder().CreatePHI(
        llvm::Type::getDoubleTy(codeGenInstance->getContext()), 2, "iftmp"
    );
    pn->addIncoming(thenV, thenBB);
    pn->addIncoming(elseV, elseBB);
    
    return pn;
}

llvm::Value* WhileStmtAST::codegen() {
    llvm::Function* function = codeGenInstance->getBuilder().GetInsertBlock()->getParent();
    
    // Create blocks for loop condition, body, and after loop
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "loopcond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "loop");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(codeGenInstance->getContext(), "afterloop");
    
    // Jump to condition block
    codeGenInstance->getBuilder().CreateBr(condBB);
    
    // Emit condition block
    codeGenInstance->getBuilder().SetInsertPoint(condBB);
    llvm::Value* condV = condition->codegen();
    if (!condV) return nullptr;
    
    // Convert condition to boolean by comparing with 0.0
    condV = codeGenInstance->getBuilder().CreateFCmpONE(
        condV, 
        llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0)), 
        "loopcond"
    );
    
    codeGenInstance->getBuilder().CreateCondBr(condV, loopBB, afterBB);
    
    // Emit loop body
    function->insert(function->end(), loopBB);
    codeGenInstance->getBuilder().SetInsertPoint(loopBB);
    
    llvm::Value* bodyV = body->codegen();
    if (!bodyV) return nullptr;
    
    // Jump back to condition
    codeGenInstance->getBuilder().CreateBr(condBB);
    
    // Emit after loop block
    function->insert(function->end(), afterBB);
    codeGenInstance->getBuilder().SetInsertPoint(afterBB);
    
    // Return a constant value for while loops
    return llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
}

llvm::Value* BlockAST::codegen() {
    llvm::Value* lastValue = nullptr;
    // Enter lexical scope for block
    codeGenInstance->enterScope();
    for (const auto& stmt : statements) {
        lastValue = stmt->codegen();
        if (!lastValue) return nullptr;
    }
    codeGenInstance->exitScope();
    // Return the last statement's value, or 0.0 if no statements
    return lastValue ? lastValue : llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
}

llvm::Value* ProgramAST::codegen() {
    llvm::Value* lastValue = nullptr;
    
    for (const auto& stmt : statements) {
        lastValue = stmt->codegen();
        if (!lastValue) return nullptr;
    }
    
    return lastValue ? lastValue : llvm::ConstantFP::get(codeGenInstance->getContext(), llvm::APFloat(0.0));
}

void initializeCodeGen(const std::string& moduleName, const std::string& sourceFile) {
    codeGenInstance = std::make_unique<CodeGen>(moduleName, sourceFile);
}

CodeGen& getCodeGen() {
    if (!codeGenInstance) {
        throw std::runtime_error("CodeGen not initialized");
    }
    return *codeGenInstance;
}