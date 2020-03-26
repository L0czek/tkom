
find_package(GTest REQUIRED)

add_executable(LexerTests tests/lexer.cc )

target_link_libraries(LexerTests Lexer ${GTEST_LIBRARIES} pthread)

add_test(NAME LexerTests COMMAND ./LexerTests)
