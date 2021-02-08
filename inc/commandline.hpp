#ifndef __COMMAND_LINE_HPP__
#define __COMMAND_LINE_HPP__

#include <boost/program_options.hpp>
#include <optional>

namespace po = boost::program_options;

class CommandLine {
	po::variables_map options;
	po::options_description desc;
	static po::options_description make_options();
	CommandLine(const po::options_description &desc) : desc(desc)
	{
	}

    public:
	void help();
	static CommandLine parse(int argc, char *argv[]);
	std::optional<std::string> getInputFile() const noexcept;
	std::optional<std::string> getOutputFile() const noexcept;
	bool runJIT() const noexcept;
	bool compileToIr() const noexcept;
	bool compileToBc() const noexcept;
	bool printIr() const noexcept;
	bool helpOpt() const noexcept;
};

#endif
