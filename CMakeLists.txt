cmake_minimum_required(VERSION 3.0)

project(TKOM)

include_directories("inc")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ggdb")


add_executable(rc
        src/main.cc
        src/token.cc
        src/lexer.cc
        src/source.cc
    )
