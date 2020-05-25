enable_testing()
find_package(GTest REQUIRED)

include_directories(tests)
add_executable(LexerTests tests/lexer.cc)

add_executable(ParserTests tests/parser.cc)

target_link_libraries(LexerTests Lexer ${GTEST_LIBRARIES} pthread)
target_link_libraries(ParserTests Parser Lexer ${GTEST_LIBRARIES} pthread)

add_test(NAME LexerTests COMMAND ./LexerTests)
add_test(NAME ParserTests COMMAND ./ParserTests)

