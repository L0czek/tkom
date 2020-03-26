cmake_minimum_required(VERSION 3.0)

project(TKOM)

include_directories("inc")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ggdb")

add_library(Lexer STATIC
        src/lexer.cc
        src/token.cc
        src/source.cc
    )

add_executable(rc src/main.cc)

target_link_libraries(rc Lexer)

enable_testing()

include(tests/CMakeLists.txt)
