#include "commandline.hpp"

#include <iostream>
#include <string>

po::options_description CommandLine::make_options()
{
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")("input-file,i", po::value<std::string>(), "set input file")(
        "output-file,o", po::value<std::string>(), "set output file")("jit", "execute compiled program")(
        "ir", "compile to llvm's IR")("bc", "compile to llvm's bytecode")("print-ir,p", "print llvm's IR");
    return desc;
}

void conflicting_options(const po::variables_map &vm, const char *opt1, const char *opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted())
        throw std::logic_error(std::string("Conflicting options '") + opt1 + "' and '" + opt2 + "'.");
}

void CommandLine::help()
{
    std::cout << desc << "\n";
}

CommandLine CommandLine::parse(int argc, char *argv[])
{
    auto desc = CommandLine::make_options();
    auto cmd = CommandLine(desc);
    po::store(po::parse_command_line(argc, argv, desc), cmd.options);
    conflicting_options(cmd.options, "jit", "print-ir");
    conflicting_options(cmd.options, "jit", "bc");
    conflicting_options(cmd.options, "jit", "ir");
    conflicting_options(cmd.options, "ir", "bc");
    conflicting_options(cmd.options, "print-ir", "bc");
    conflicting_options(cmd.options, "print-ir", "it");
    conflicting_options(cmd.options, "output-file", "print-ir");
    conflicting_options(cmd.options, "output-file", "jit");
    return cmd;
}

std::optional<std::string> CommandLine::getInputFile() const noexcept
{
    if (options.count("input-file")) {
        return options["input-file"].as<std::string>();
    } else {
        return {};
    }
}

std::optional<std::string> CommandLine::getOutputFile() const noexcept
{
    if (options.count("output-file")) {
        return options["output-file"].as<std::string>();
    } else {
        return {};
    }
}

bool CommandLine::runJIT() const noexcept
{
    return options.count("jit");
}

bool CommandLine::compileToIr() const noexcept
{
    return options.count("ir");
}

bool CommandLine::compileToBc() const noexcept
{
    return options.count("bc");
}

bool CommandLine::printIr() const noexcept
{
    return options.count("print-ir");
}

bool CommandLine::helpOpt() const noexcept
{
    return options.count("help");
}
