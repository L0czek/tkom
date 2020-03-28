#include <iostream>

#include "source.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    auto lexer = Lexer::from_source(Source::from_stdin());
    Parser parser;
    try{ 
        parser.attach_lexer(std::move(lexer));
        auto ast = parser.parse_Program();
        int a = 1;
    } catch (LexerException e) {
        std::wcout << e.message() << L"\n";
    } catch (ParserException e) {
        std::wcout << e.message() << L"\n";
    }
}
