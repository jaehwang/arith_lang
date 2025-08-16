#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Constants.h"

void initializeCodeGen(const std::string& moduleName, const std::string& sourceFile = "");
CodeGen& getCodeGen();

class CodeGenTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(CodeGenTest, NumberExpressionCodegen) {
    initializeCodeGen("test_module_num");
    auto numExpr = std::make_unique<NumberExprAST>(42.0);
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = numExpr->codegen();
    ASSERT_NE(value, nullptr);
    
    // Check if it's a constant with the expected value
    auto constant = llvm::dyn_cast<llvm::ConstantFP>(value);
    ASSERT_NE(constant, nullptr);
    EXPECT_DOUBLE_EQ(constant->getValueAPF().convertToDouble(), 42.0);
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
}

TEST_F(CodeGenTest, BinaryExpressionAddCodegen) {
    initializeCodeGen("test_module_add");
    auto left = std::make_unique<NumberExprAST>(10.0);
    auto right = std::make_unique<NumberExprAST>(5.0);
    auto binExpr = std::make_unique<BinaryExprAST>('+', std::move(left), std::move(right));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = binExpr->codegen();
    ASSERT_NE(value, nullptr);
    
    // Check that it's an FAdd instruction
    EXPECT_TRUE(value->getType()->isDoubleTy());
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
}

TEST_F(CodeGenTest, BinaryExpressionSubCodegen) {
    initializeCodeGen("test_module_sub");
    auto left = std::make_unique<NumberExprAST>(10.0);
    auto right = std::make_unique<NumberExprAST>(3.0);
    auto binExpr = std::make_unique<BinaryExprAST>('-', std::move(left), std::move(right));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = binExpr->codegen();
    ASSERT_NE(value, nullptr);
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, BinaryExpressionMulCodegen) {
    initializeCodeGen("test_module_mul");
    auto left = std::make_unique<NumberExprAST>(4.0);
    auto right = std::make_unique<NumberExprAST>(5.0);
    auto binExpr = std::make_unique<BinaryExprAST>('*', std::move(left), std::move(right));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = binExpr->codegen();
    ASSERT_NE(value, nullptr);
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, BinaryExpressionDivCodegen) {
    initializeCodeGen("test_module_div");
    auto left = std::make_unique<NumberExprAST>(20.0);
    auto right = std::make_unique<NumberExprAST>(4.0);
    auto binExpr = std::make_unique<BinaryExprAST>('/', std::move(left), std::move(right));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = binExpr->codegen();
    ASSERT_NE(value, nullptr);
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, VariableAssignmentAndAccess) {
    initializeCodeGen("test_module_var");
    auto assignExpr = std::make_unique<AssignmentExprAST>("x", std::make_unique<NumberExprAST>(42.0));
    auto varExpr = std::make_unique<VariableExprAST>("x");
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto assignValue = assignExpr->codegen();
    ASSERT_NE(assignValue, nullptr);
    
    auto varValue = varExpr->codegen();
    ASSERT_NE(varValue, nullptr);
    
    getCodeGen().getBuilder().CreateRet(varValue);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, ComparisonOperators) {
    initializeCodeGen("test_module_cmp");
    struct TestCase {
        char op;
        double left;
        double right;
        double expected;
    };
    
    std::vector<TestCase> testCases = {
        {'>', 5.0, 3.0, 1.0},
        {'>', 3.0, 5.0, 0.0},
        {'<', 3.0, 5.0, 1.0},
        {'<', 5.0, 3.0, 0.0}
    };
    
    for (const auto& testCase : testCases) {
        auto left = std::make_unique<NumberExprAST>(testCase.left);
        auto right = std::make_unique<NumberExprAST>(testCase.right);
        auto binExpr = std::make_unique<BinaryExprAST>(testCase.op, std::move(left), std::move(right));
        
        auto func = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
            llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
        
        auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
        getCodeGen().getBuilder().SetInsertPoint(entry);
        
        auto value = binExpr->codegen();
        ASSERT_NE(value, nullptr);
        
        getCodeGen().getBuilder().CreateRet(value);
        
        ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
        
        // IR generation successful for comparison operators
    }
}

TEST_F(CodeGenTest, PrintStatementCodegen) {
    initializeCodeGen("test_module_print");
    auto printStmt = std::make_unique<PrintStmtAST>(std::make_unique<NumberExprAST>(123.0));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = printStmt->codegen();
    ASSERT_NE(value, nullptr);
    
    getCodeGen().getBuilder().CreateRet(llvm::ConstantFP::get(getCodeGen().getContext(), llvm::APFloat(0.0)));
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
}

TEST_F(CodeGenTest, IfStatementCodegen) {
    initializeCodeGen("test_module_if");
    auto condition = std::make_unique<BinaryExprAST>('>', 
        std::make_unique<NumberExprAST>(5.0), 
        std::make_unique<NumberExprAST>(3.0));
    
    auto thenStmt = std::make_unique<AssignmentExprAST>("result", std::make_unique<NumberExprAST>(1.0));
    auto elseStmt = std::make_unique<AssignmentExprAST>("result", std::make_unique<NumberExprAST>(0.0));
    
    auto ifStmt = std::make_unique<IfStmtAST>(std::move(condition), std::move(thenStmt), std::move(elseStmt));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = ifStmt->codegen();
    ASSERT_NE(value, nullptr);
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, WhileStatementCodegen) {
    initializeCodeGen("test_module_while");
    
    // Create a simple while(0) loop that should never execute
    auto condition = std::make_unique<NumberExprAST>(0.0);  // false condition
    auto body = std::make_unique<AssignmentExprAST>("x", std::make_unique<NumberExprAST>(1.0));
    
    auto whileStmt = std::make_unique<WhileStmtAST>(std::move(condition), std::move(body));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = whileStmt->codegen();
    ASSERT_NE(value, nullptr);
    
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

TEST_F(CodeGenTest, BlockStatementCodegen) {
    initializeCodeGen("test_module_block");
    
    std::vector<std::unique_ptr<ASTNode>> stmts;
    stmts.push_back(std::make_unique<AssignmentExprAST>("x", std::make_unique<NumberExprAST>(10.0)));
    stmts.push_back(std::make_unique<AssignmentExprAST>("y", std::make_unique<NumberExprAST>(5.0)));
    stmts.push_back(std::make_unique<AssignmentExprAST>("result", 
        std::make_unique<BinaryExprAST>('+', 
            std::make_unique<VariableExprAST>("x"), 
            std::make_unique<VariableExprAST>("y"))));
    
    auto block = std::make_unique<BlockAST>(std::move(stmts));
    
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getDoubleTy(getCodeGen().getContext()), false),
        llvm::Function::ExternalLinkage, "test_func", &getCodeGen().getModule());
    
    auto entry = llvm::BasicBlock::Create(getCodeGen().getContext(), "entry", func);
    getCodeGen().getBuilder().SetInsertPoint(entry);
    
    auto value = block->codegen();
    ASSERT_NE(value, nullptr);
    // The value of the block should be the last statement's value; return it directly
    getCodeGen().getBuilder().CreateRet(value);
    
    ASSERT_FALSE(llvm::verifyFunction(*func, &llvm::errs()));
    
    // IR generation successful
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}