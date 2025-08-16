#include <gtest/gtest.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/codegen.h"
#include "../include/type_check.h"
#include "../include/ast.h"
#include <memory>
#include <stdexcept>

void initializeCodeGen(const std::string& moduleName, const std::string& sourceFile = "");
CodeGen& getCodeGen();

class MutabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset for each test
    }
    
    void TearDown() override {
        // Cleanup if needed
    }
    
    // Helper to parse program and get assignment AST
    std::unique_ptr<ASTNode> parseProgram(const std::string& input) {
        Lexer lexer(input, "test.k");
        Parser parser(lexer);
        return parser.parseProgram();
    }
    
    // Helper to get first assignment from program
    AssignmentExprAST* getFirstAssignment(ASTNode* program) {
        auto* prog = dynamic_cast<ProgramAST*>(program);
        if (!prog || prog->getStatements().empty()) return nullptr;
        return dynamic_cast<AssignmentExprAST*>(prog->getStatements()[0].get());
    }
};

// Test Category 1: MutabilityLexerTests - mut keyword tokenization
class MutabilityLexerTests : public MutabilityTest {};

TEST_F(MutabilityLexerTests, RecognizesMutKeyword) {
    Lexer lexer("mut", "test.k");
    Token token = lexer.getNextToken();
    
    EXPECT_EQ(token.type, TOK_MUT);
    EXPECT_EQ(token.value, "mut");
}

TEST_F(MutabilityLexerTests, MutKeywordInContext) {
    Lexer lexer("mut x = 5;", "test.k");
    
    Token mut_token = lexer.getNextToken();
    EXPECT_EQ(mut_token.type, TOK_MUT);
    
    Token id_token = lexer.getNextToken();
    EXPECT_EQ(id_token.type, TOK_IDENTIFIER);
    EXPECT_EQ(id_token.value, "x");
    
    Token assign_token = lexer.getNextToken();
    EXPECT_EQ(assign_token.type, TOK_ASSIGN);
}

// Test Category 2: MutabilityParserTests - mut declaration parsing
class MutabilityParserTests : public MutabilityTest {};

TEST_F(MutabilityParserTests, ParseMutableDeclaration) {
    auto program = parseProgram("mut x = 42;");
    AssignmentExprAST* assignment = getFirstAssignment(program.get());
    
    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->getVarName(), "x");
    EXPECT_TRUE(assignment->isMutableDeclaration());
    EXPECT_EQ(assignment->getAssignmentType(), AssignmentType::DECLARATION);
}

TEST_F(MutabilityParserTests, ParseImmutableDeclaration) {
    auto program = parseProgram("x = 42;");
    AssignmentExprAST* assignment = getFirstAssignment(program.get());
    
    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->getVarName(), "x");
    EXPECT_FALSE(assignment->isMutableDeclaration());
    EXPECT_EQ(assignment->getAssignmentType(), AssignmentType::DECLARATION);
}

TEST_F(MutabilityParserTests, ParseMutableWithExpression) {
    auto program = parseProgram("mut y = 1 + 2 * 3;");
    AssignmentExprAST* assignment = getFirstAssignment(program.get());
    
    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->getVarName(), "y");
    EXPECT_TRUE(assignment->isMutableDeclaration());
}

TEST_F(MutabilityParserTests, ErrorOnMutWithoutIdentifier) {
    Lexer lexer("mut = 42;", "test.k");
    Parser parser(lexer);
    
    EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST_F(MutabilityParserTests, ErrorOnMutWithoutAssignment) {
    Lexer lexer("mut x 42;", "test.k");
    Parser parser(lexer);
    
    EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}

// Test Category 3: MutabilityTypeCheckTests - mutability validation
class MutabilityTypeCheckTests : public MutabilityTest {};

TEST_F(MutabilityTypeCheckTests, ValidMutableDeclaration) {
    auto program = parseProgram("mut x = 42;");
    
    // Type checking should not throw for valid mutable declaration
    EXPECT_NO_THROW(typeCheck(program.get()));
}

TEST_F(MutabilityTypeCheckTests, ValidImmutableDeclaration) {
    auto program = parseProgram("x = 42;");
    
    // Type checking should not throw for valid immutable declaration  
    EXPECT_NO_THROW(typeCheck(program.get()));
}

TEST_F(MutabilityTypeCheckTests, MutableWithStringLiteral) {
    auto program = parseProgram("mut x = \"hello\";");
    
    // Type checking should not throw for string assignment to mutable var
    EXPECT_NO_THROW(typeCheck(program.get()));
}

// Test Category 4: MutabilityCodeGenTests - variable storage with mutability
class MutabilityCodeGenTests : public MutabilityTest {};

TEST_F(MutabilityCodeGenTests, CreateMutableVariable) {
    initializeCodeGen("test_module_mut_var", "test.k");
    auto& codegen = getCodeGen();
    codegen.enterScope();
    
    auto* alloca = codegen.declareVariable("x", true);
    
    ASSERT_NE(alloca, nullptr);
    EXPECT_TRUE(codegen.canReassign("x"));
    EXPECT_TRUE(codegen.hasCurrentSymbol("x"));
    EXPECT_TRUE(codegen.isCurrentSymbolMutable("x"));
}

TEST_F(MutabilityCodeGenTests, CreateImmutableVariable) {
    initializeCodeGen("test_module_immut_var", "test.k");
    auto& codegen = getCodeGen();
    codegen.enterScope();
    
    auto* alloca = codegen.declareVariable("x", false);
    
    ASSERT_NE(alloca, nullptr);
    EXPECT_FALSE(codegen.canReassign("x"));
    EXPECT_TRUE(codegen.hasCurrentSymbol("x"));
    EXPECT_FALSE(codegen.isCurrentSymbolMutable("x"));
}

TEST_F(MutabilityCodeGenTests, VariableLookup) {
    initializeCodeGen("test_module_lookup", "test.k");
    auto& codegen = getCodeGen();
    codegen.enterScope();
    
    auto* alloca1 = codegen.declareVariable("mutable_var", true);
    auto* alloca2 = codegen.declareVariable("immutable_var", false);
    
    EXPECT_EQ(codegen.getVariable("mutable_var"), alloca1);
    EXPECT_EQ(codegen.getVariable("immutable_var"), alloca2);
    EXPECT_EQ(codegen.getVariable("nonexistent"), nullptr);
}

TEST_F(MutabilityCodeGenTests, ScopeManagement) {
    initializeCodeGen("test_module_scope", "test.k");
    auto& codegen = getCodeGen();
    
    // Outer scope
    codegen.enterScope();
    codegen.declareVariable("outer_var", true);
    EXPECT_TRUE(codegen.hasCurrentSymbol("outer_var"));
    
    // Inner scope
    codegen.enterScope();
    codegen.declareVariable("inner_var", false);
    EXPECT_TRUE(codegen.hasCurrentSymbol("inner_var"));
    EXPECT_TRUE(codegen.hasNearestSymbol("outer_var")); // Can see outer scope
    
    // Exit inner scope
    codegen.exitScope();
    EXPECT_FALSE(codegen.hasCurrentSymbol("inner_var")); // Inner var gone
    EXPECT_TRUE(codegen.hasCurrentSymbol("outer_var"));  // Outer var still there
}

TEST_F(MutabilityCodeGenTests, VariableShadowing) {
    initializeCodeGen("test_module_shadow", "test.k");
    auto& codegen = getCodeGen();
    
    codegen.enterScope();
    auto* outer_alloca = codegen.declareVariable("x", true);
    
    codegen.enterScope();
    auto* inner_alloca = codegen.declareVariable("x", false); // Shadow with immutable
    
    // Current scope should see the inner variable
    EXPECT_EQ(codegen.getCurrentAlloca("x"), inner_alloca);
    EXPECT_FALSE(codegen.isCurrentSymbolMutable("x"));
    
    // Nearest lookup should also see inner variable
    EXPECT_EQ(codegen.getNearestAlloca("x"), inner_alloca);
    
    codegen.exitScope();
    
    // Back to outer scope
    EXPECT_EQ(codegen.getCurrentAlloca("x"), outer_alloca);
    EXPECT_TRUE(codegen.isCurrentSymbolMutable("x"));
}

// Test Category 5: MutabilityIntegrationTests - end-to-end behavior
class MutabilityIntegrationTests : public MutabilityTest {};

TEST_F(MutabilityIntegrationTests, MutableVariableCodeGeneration) {
    auto program = parseProgram("mut x = 42;");
    
    // Should compile without errors
    EXPECT_NO_THROW(typeCheck(program.get()));
    
    // For now, skip direct codegen() calls to avoid conflicts with global CodeGen
    // The integration will be tested via .k files using test_runner.sh
}

TEST_F(MutabilityIntegrationTests, ImmutableVariableCodeGeneration) {
    auto program = parseProgram("x = 42;");
    
    // Should compile without errors
    EXPECT_NO_THROW(typeCheck(program.get()));
    
    // For now, skip direct codegen() calls to avoid conflicts with global CodeGen
    // The integration will be tested via .k files using test_runner.sh
}

TEST_F(MutabilityIntegrationTests, MultipleVariableDeclarations) {
    auto program = parseProgram(R"(
        mut x = 10;
        y = 20;
        mut z = 30;
    )");
    
    EXPECT_NO_THROW(typeCheck(program.get()));
    
    // For now, skip direct codegen() calls to avoid conflicts with global CodeGen
    // The integration will be tested via .k files using test_runner.sh
}

// Test Category 6: MutabilityErrorTests - error handling
class MutabilityErrorTests : public MutabilityTest {};

TEST_F(MutabilityErrorTests, InvalidMutSyntax) {
    // Missing variable name after mut
    Lexer lexer1("mut;", "test.k");
    Parser parser1(lexer1);
    EXPECT_THROW(parser1.parseProgram(), std::runtime_error);
    
    // Missing assignment after variable name
    Lexer lexer2("mut x;", "test.k");
    Parser parser2(lexer2);
    EXPECT_THROW(parser2.parseProgram(), std::runtime_error);
    
    // Missing semicolon
    Lexer lexer3("mut x = 5", "test.k");
    Parser parser3(lexer3);
    EXPECT_THROW(parser3.parseProgram(), std::runtime_error);
}

TEST_F(MutabilityErrorTests, CanShadowHelperTest) {
    initializeCodeGen("test_module_shadow_helper", "test.k");
    auto& codegen = getCodeGen();
    codegen.enterScope();
    
    // Language allows shadowing for any variable
    EXPECT_TRUE(codegen.canShadow("nonexistent"));
    
    codegen.declareVariable("x", true);
    EXPECT_TRUE(codegen.canShadow("x"));
    
    codegen.declareVariable("y", false);
    EXPECT_TRUE(codegen.canShadow("y"));
}

// Parameterized tests for comprehensive coverage
class MutabilityParameterizedTests : public MutabilityTest, 
                                    public ::testing::WithParamInterface<std::tuple<std::string, bool, bool>> {};

TEST_P(MutabilityParameterizedTests, ParseVariousDeclarations) {
    auto [code, should_be_mutable, should_succeed] = GetParam();
    
    if (should_succeed) {
        auto program = parseProgram(code);
        AssignmentExprAST* assignment = getFirstAssignment(program.get());
        
        ASSERT_NE(assignment, nullptr);
        EXPECT_EQ(assignment->isMutableDeclaration(), should_be_mutable);
        EXPECT_NO_THROW(typeCheck(program.get()));
        // Skip codegen() call to avoid conflicts with global CodeGen
    } else {
        Lexer lexer(code, "test.k");
        Parser parser(lexer);
        EXPECT_THROW(parser.parseProgram(), std::runtime_error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    MutabilityDeclarations,
    MutabilityParameterizedTests,
    ::testing::Values(
        // Valid cases: (code, is_mutable, should_succeed)
        std::make_tuple("mut x = 1;", true, true),
        std::make_tuple("x = 1;", false, true),
        std::make_tuple("mut y = 1 + 2;", true, true),
        std::make_tuple("z = 3 * 4;", false, true),
        std::make_tuple("mut a = -5;", true, true),
        std::make_tuple("b = (1 + 2) * 3;", false, true),
        
        // Invalid cases: (code, irrelevant, should_succeed)
        std::make_tuple("mut;", false, false),
        std::make_tuple("mut = 1;", false, false),
        std::make_tuple("mut x", false, false),
        std::make_tuple("mut x =;", false, false),
        std::make_tuple("mut 123 = 1;", false, false)
    )
);
