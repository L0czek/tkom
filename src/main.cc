#include <iostream>

#include "source.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "print.hpp"
#include "semantic.hpp"

int main() {
    auto lexer = Lexer::from_source(Source::from_stdin());
    Parser parser;
    try{ 
        parser.attach_lexer(std::move(lexer));
        auto ast = parser.parse_Program();
        PrintVisitor visitor{};
        ast->accept(visitor);
        std::wcout << visitor.result() << L"\n";
        lexer = parser.detach_lexer();
        auto source = lexer->change_source(nullptr);
        analyse(ast, std::move(source));
        int a = 1;
    
    } catch (LexerException e) {
        std::wcout << e.message() << L"\n";
    } catch (ParserException e) {
        std::wcout << e.message() << L"\n";
    } catch (SemanticException e) {
        std::wcout << e.message() << L"\n";
    }
}
