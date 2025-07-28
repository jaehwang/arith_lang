cmake_minimum_required(VERSION 3.16)
project(ArithLang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

# Download and build Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(include)

set(SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    src/main.cpp
)

set(LEXER_TEST_SOURCES
    src/lexer.cpp
    tests/test_lexer.cpp
)

set(PARSER_TEST_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    tests/test_parser.cpp
)

set(INTEGRATION_TEST_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    tests/test_integration.cpp
)

add_executable(arithc ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(arithc ${llvm_libs})

# Build tests with downloaded GTest
add_executable(test_lexer ${LEXER_TEST_SOURCES})
target_link_libraries(test_lexer gtest_main)

add_executable(test_parser ${PARSER_TEST_SOURCES})
target_link_libraries(test_parser ${llvm_libs} gtest_main)

add_executable(test_integration ${INTEGRATION_TEST_SOURCES})
target_link_libraries(test_integration ${llvm_libs} gtest_main)

enable_testing()
add_test(NAME LexerTests COMMAND test_lexer)
add_test(NAME ParserTests COMMAND test_parser)
add_test(NAME IntegrationTests COMMAND test_integration)