cmake_minimum_required(VERSION 3.0)

project(TKOM)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("inc")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ggdb -Wall -Wextra")

#include_directories(/usr/lib/llvm-9/include)
add_definitions(${LLVM_DEFINITIONS})

#link_directories(/usr/lib/llvm-9/lib/)
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

add_library(CommandLine STATIC
        src/commandline.cc
    )

add_executable(rc src/main.cc)
target_link_libraries(Parser Common)
target_link_libraries(Analyser Common)
target_link_libraries(CommandLine boost_program_options)
target_link_libraries(LLVMBackend  LLVM-9)

target_link_libraries(LLVMBackend
        z
        dl
        pthread
        ncurses
    )

target_link_libraries(rc CommandLine Lexer Parser Analyser LLVMBackend)

include(tests/CMakeLists.txt)
