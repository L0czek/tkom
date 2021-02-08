#include <iostream>

#include "source.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "print.hpp"
#include "semantic.hpp"
#include "backend.hpp"
#include "commandline.hpp"
#include <boost/exception/all.hpp>

int main(int argc, char* argv[]) {
    
    
    try {
        auto options = CommandLine::parse(argc, argv);

        if (options.helpOpt()) {
            options.help();
            return 0;
        }

        std::unique_ptr<Source> source;
        if (options.getInputFile()) {
            source = Source::from_file(*options.getInputFile());
        } else {
            source = Source::from_stdin();
        }
        
        auto lexer = Lexer::from_source(std::move(source));
        Parser parser;
        parser.attach_lexer(std::move(lexer));
        auto program = parser.parse();
        source = parser.detach_lexer()->change_source();
        analyse(program, std::move(source));
        auto compiled = compile(program);
        
        if (options.getOutputFile()) {
            if (options.compileToIr()) {
                compiled->save_ir(*options.getOutputFile());
            } else if (options.compileToBc()) {
                compiled->save_bc(*options.getOutputFile());
            }
        } else {
            if (options.printIr()) {
                compiled->print_ir();
            } else if (options.runJIT()) {
                return compiled->execute();
            }
        }
    } catch (const SourceException& e) {
        std::cerr << e.message() << L"\n";
    } catch (const LexerException& e) {
        std::wcerr << e.message() << L"\n";
    } catch (const ParserException& e) {
        std::wcerr << e.message() << L"\n";
    } catch (const SemanticException& e) {
        std::wcerr << e.message() << L"\n";
    } catch (const CompilerException& e) {
        std::wcerr << e.message() << L"\n";
    } catch (const std::exception& e) {
        std::cerr << "Commandline error: " << e.what() << "\n";
    } 
}
