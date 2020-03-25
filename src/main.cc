#include <iostream>

#include "source.hpp"
#include "lexer.hpp"

int main() {
    Lexer lexer(Source::from_stdin());
    try{ 
        auto token = lexer.next();
        while (!is_eof(token)) {
            std::wcout << repr(token) << L"\n";
            token = lexer.next();
        }
        std::wcout << repr(token) << L"\n";
    } catch (LexerException e) {
        std::wcout << e.message() << L"\n";
    }
}
