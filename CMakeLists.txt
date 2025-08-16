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

# Public headers
include_directories(include)

# Core static library (compile once, link everywhere)
add_library(arith_core STATIC
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    src/type_check.cpp
  src/parse_error_reporting.cpp
)
target_include_directories(arith_core PUBLIC include)

# LLVM components (used by targets that actually require codegen)
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Main executable
add_executable(arithc src/main.cpp)
target_link_libraries(arithc arith_core ${llvm_libs})

# Tests
enable_testing()

add_executable(test_lexer tests/test_lexer.cpp)
target_link_libraries(test_lexer arith_core gtest_main)
add_test(NAME LexerTests COMMAND test_lexer)

add_executable(test_parser tests/test_parser.cpp)
target_link_libraries(test_parser arith_core ${llvm_libs} gtest_main)
add_test(NAME ParserTests COMMAND test_parser)

add_executable(test_parser_negative tests/test_parser_negative.cpp)
target_link_libraries(test_parser_negative arith_core ${llvm_libs} gtest_main gmock_main)
add_test(NAME ParserNegativeTests COMMAND test_parser_negative)

add_executable(test_codegen tests/test_codegen.cpp)
target_link_libraries(test_codegen arith_core ${llvm_libs} gtest_main)
add_test(NAME CodeGenTests COMMAND test_codegen)

add_executable(test_integration tests/test_integration.cpp)
target_link_libraries(test_integration arith_core ${llvm_libs} gtest_main)
add_test(NAME IntegrationTests COMMAND test_integration)

# Phase 1: Location and ParseError tests
add_executable(test_source_location tests/test_source_location.cpp)
target_link_libraries(test_source_location arith_core gtest_main gmock)
add_test(NAME SourceLocationTests COMMAND test_source_location)

add_executable(test_parser_error_helpers tests/test_parser_error_helpers.cpp)
target_link_libraries(test_parser_error_helpers arith_core ${llvm_libs} gtest_main gmock)
add_test(NAME ParserErrorHelpersTests COMMAND test_parser_error_helpers)

add_executable(test_error_display tests/test_error_display.cpp)
target_link_libraries(test_error_display arith_core gtest_main gmock)
add_test(NAME ErrorDisplayTests COMMAND test_error_display)

add_executable(test_integration_error_reporting tests/test_integration_error_reporting.cpp)
target_link_libraries(test_integration_error_reporting arith_core ${llvm_libs} gtest_main gmock)
add_test(NAME IntegrationErrorReportingTests COMMAND test_integration_error_reporting)

# Mutability tests
add_executable(test_mutability tests/test_mutability.cpp)
target_link_libraries(test_mutability arith_core ${llvm_libs} gtest_main gmock)
add_test(NAME MutabilityTests COMMAND test_mutability)
