#ifndef __SOURCE_HPP__
#define __SOURCE_HPP__

#include <iostream>
#include <fstream>
#include <optional>
#include <memory>
#include <sstream>
#include <unordered_map>

struct Position {
	std::size_t stream_position;
	std::size_t line_number;
	std::size_t column_number;
};

std::wstring to_wstring(const Position &position);

class Source {
	Position current_position;
	std::unordered_map<std::size_t, Position> line_position;
	Source(const Source &) = delete;

    protected:
	Source();
	void update_position(wchar_t ch);

    public:
	virtual std::optional<wchar_t> next() noexcept = 0;
	const Position &get_position() const noexcept;
	virtual std::wstring input_between(const Position &start, const Position &end) = 0;
	std::wstring get_lines(std::size_t from, std::size_t to);

	virtual ~Source();

	static void report_error(const std::string &msg);
	static std::unique_ptr<Source> from_file(const std::string &path);
	static std::unique_ptr<Source> from_stdin();
	static std::unique_ptr<Source> from_wstring(const std::wstring &str);
};

class FileSource : public Source {
	std::wfstream file;
	FileSource(const FileSource &) = delete;

    public:
	FileSource(const std::string &path);
	~FileSource();

	std::optional<wchar_t> next() noexcept override;
	std::wstring input_between(const Position &start, const Position &end) override;
};

class StdInSource : public Source {
	std::wstring source_code;
	std::wistream source_stream;
	StdInSource(const StdInSource &) = delete;

    public:
	StdInSource();
	~StdInSource();

	std::optional<wchar_t> next() noexcept override;
	std::wstring input_between(const Position &start, const Position &end) override;
};

class StringSource : public Source {
	std::wstringstream source_stream;

    public:
	StringSource(const std::wstring &source);
	~StringSource();

	std::optional<wchar_t> next() noexcept override;
	std::wstring input_between(const Position &start, const Position &end) override;
};

class SourceException : public std::runtime_error {
	std::string msg;

    public:
	SourceException(const std::string &msg) : std::runtime_error("Source Exception"), msg(msg)
	{
	}
	const std::string &message() const noexcept
	{
		return msg;
	}
	const char *what() const noexcept override
	{
		return msg.c_str();
	}
};

#endif
