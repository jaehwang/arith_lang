#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "ast.h"
#include "type_check.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "parse_error_reporting.h"

void initializeCodeGen(const std::string& moduleName, const std::string& sourceFile = "");
CodeGen& getCodeGen();

struct CompilerOptions {
    std::string inputFile;
    std::string outputFile;
};

void printUsage(const char* programName) {
    std::cout << "ArithLang - LLVM 기반 산술 표현식 컴파일러\n\n";
    std::cout << "사용법:\n";
    std::cout << "  " << programName << " <입력파일>\n";
    std::cout << "  " << programName << " -o <출력파일> <입력파일>\n";
    std::cout << "  " << programName << " <입력파일> -o <출력파일>\n\n";
    std::cout << "옵션:\n";
    std::cout << "  -o <파일>    LLVM IR을 지정된 파일에 저장 (기본값: a.ll)\n\n";
    std::cout << "예제:\n";
    std::cout << "  " << programName << " input.k                 # a.ll로 출력\n";
    std::cout << "  " << programName << " -o output.ll input.k    # output.ll로 출력\n";
    std::cout << "  " << programName << " input.k -o output.ll    # output.ll로 출력\n";
    std::cout << "입력 파일은 .k 확장자를 사용합니다.\n";
}

std::string pathToModuleID(const std::string& path) {
    std::string result = path;
    
    // .k 확장자 제거
    if (result.length() >= 2 && result.substr(result.length() - 2) == ".k") {
        result = result.substr(0, result.length() - 2);
    }
    
    // 경로 구분자를 점으로 변경
    for (char& c : result) {
        if (c == '/' || c == '\\') {
            c = '.';
        }
    }
    
    // 시작이 점이면 제거
    if (!result.empty() && result[0] == '.') {
        result = result.substr(1);
    }
    
    return result;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("파일을 열 수 없습니다: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

CompilerOptions parseCommandLine(int argc, char* argv[]) {
    CompilerOptions options;
    
    if (argc == 2) {
        // gcc와 같은 동작: -o 없으면 현재 디렉토리에 'a.ll' 생성
        options.inputFile = argv[1];
        options.outputFile = "a.ll";
    } else if (argc == 4) {
        if (std::string(argv[1]) == "-o") {
            // -o 옵션으로 출력 파일 지정: arithc -o output.ll input.k
            options.outputFile = argv[2];
            options.inputFile = argv[3];
        } else if (std::string(argv[2]) == "-o") {
            // -o 옵션이 뒤에: arithc input.k -o output.ll
            options.inputFile = argv[1];
            options.outputFile = argv[3];
        } else {
            printUsage(argv[0]);
            throw std::runtime_error("잘못된 명령행 인자");
        }
    } else {
        printUsage(argv[0]);
        throw std::runtime_error("잘못된 명령행 인자");
    }
    
    if (options.inputFile.length() < 2 || 
        options.inputFile.substr(options.inputFile.length() - 2) != ".k") {
        throw std::runtime_error("입력 파일은 .k 확장자를 사용해야 합니다");
    }
    
    return options;
}

void setupLLVMFunction(const std::string& inputFile) {
    std::string moduleID = pathToModuleID(inputFile);
    initializeCodeGen(moduleID, inputFile);
    
    auto& codeGen = getCodeGen();
    
    // Make main return i32 for proper process exit code (0 on success)
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(codeGen.getContext()), false);
    llvm::Function* mainFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "main", codeGen.getModule());
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(
        codeGen.getContext(), "entry", mainFunc);
    codeGen.getBuilder().SetInsertPoint(entry);
}

void compileSource(const std::string& input, const std::string& filename) {
    // Ensure lexer knows the actual filename for accurate error reporting
    Lexer lexer(input, filename);
    Parser parser(lexer);
    
    // Parse entire program as single AST
    auto programAST = parser.parseProgram();
    if (!programAST) {
        throw std::runtime_error("프로그램 파싱 실패");
    }

    // 타입 체크 단계 추가
    typeCheck(programAST.get(), filename);

    // Generate IR for entire program
    llvm::Value* result = programAST->codegen();
    if (!result) {
        throw std::runtime_error("코드 생성 실패");
    }
    
    // 함수 종료 처리: return 0 (i32)
    auto& codeGen = getCodeGen();
    llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(codeGen.getContext()), 0);
    codeGen.getBuilder().CreateRet(zero);
}

void saveIRToFile(const std::string& outputFile) {
    auto& codeGen = getCodeGen();
    
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("출력 파일을 열 수 없습니다: " + outputFile);
    }
    
    std::string irString;
    llvm::raw_string_ostream stream(irString);
    codeGen.getModule().print(stream, nullptr);
    stream.flush();
    
    outFile << irString;
    outFile.close();
}

int main(int argc, char* argv[]) {
    try {
        // 명령행 처리
        CompilerOptions options = parseCommandLine(argc, argv);
        
        // 입력 파일 읽기
        std::string input = readFile(options.inputFile);
        
        // LLVM 함수 설정
        setupLLVMFunction(options.inputFile);
        
        // 소스 컴파일
        compileSource(input, options.inputFile);
        
        // IR 저장
        saveIRToFile(options.outputFile);
        
        std::cout << "IR이 생성되었습니다: " << options.outputFile << std::endl;
        
    } catch (const std::exception& e) {
        // Try to detect ParseError via dynamic_cast
        try {
            throw; // rethrow
        } catch (const ParseError& pe) {
            printParseError(pe, readFile(pe.loc.file));
            return 1;
        } catch (const std::exception& ex) {
            std::cerr << "오류: " << ex.what() << std::endl;
            return 1;
        }
    }
    
    return 0;
}
