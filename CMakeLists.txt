cmake_minimum_required(VERSION 3.0)

project(TKOM)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories("inc")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ggdb -Wall -Wextra")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(Lexer STATIC
        src/lexer.cc
        src/token.cc
        src/source.cc
    )

add_library(Parser STATIC
        src/parser.cc
        src/node.cc
        src/print.cc
    )

add_library(Analyser STATIC
        src/semantic.cc
    )

add_library(LLVMBackend STATIC
        src/backend.cc
    )

add_library(Common STATIC
        src/common.cc
    )

add_executable(rc src/main.cc)

llvm_map_components_to_libnames(llvm_libs support core irreader scalaropts bitwriter executionengine target x86info x86desc x86utils)
target_link_libraries(Parser Common)
target_link_libraries(Analyser Common)
target_link_libraries(LLVMBackend ${llvm_libs})

target_link_libraries(rc Lexer Parser Analyser LLVMBackend)

enable_testing()

include(tests/CMakeLists.txt)

