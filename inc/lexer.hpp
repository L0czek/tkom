#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include "token.hpp"
#include "source.hpp"
#include "common.hpp"
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

class Lexer {
	std::unique_ptr<Source> source;
	std::optional<wchar_t> ch_opt;
	std::locale locale;
	Position position;

	static const std::unordered_map<std::wstring, TokenType> keywords;
	static const std::unordered_set<wchar_t> operator_chars;

	Token keyword_or_identifier();
	Token operator_lexem();
	Token int_const();
	Token string_const();
	wchar_t escape_char(wchar_t ch);

	template <typename... Options> Token choose_operator_on(wchar_t chr, TokenType on_match, Options &&... options);
	Token choose_operator_on(TokenType on_mismatch);

	bool skip_space();
	bool skip_comment();

	[[noreturn]] void report_error(const Position &error_position, const std::wstring &error_msg, wchar_t bad_char);
	[[noreturn]] void report_error(const Position &error_position, const std::wstring &error_msg,
				       const std::wstring &bad_lexem);

    public:
	Lexer(std::unique_ptr<Source> source = nullptr);

	Token next();

	std::unique_ptr<Source> change_source(std::unique_ptr<Source> source = nullptr) noexcept;
	std::wstring source_between(const Position &start, const Position &end);
	std::wstring get_lines(std::size_t from, std::size_t to);

	static std::unique_ptr<Lexer> from_source(std::unique_ptr<Source> source);
};

class LexerException : public std::runtime_error {
	std::wstring msg;
	std::string ascii_msg;

    public:
	LexerException(const std::wstring &error)
		: std::runtime_error("LexerException"), msg(error), ascii_msg(to_ascii_string(msg))
	{
	}
	const char *what() const noexcept override
	{
		return ascii_msg.c_str();
	}
	const std::wstring &message() const noexcept
	{
		return msg;
	}
};

template <typename... Options> Token Lexer::choose_operator_on(wchar_t chr, TokenType on_match, Options &&... options)
{
	if (ch_opt && *ch_opt == chr) {
		ch_opt = source->next();
		return make_token(on_match, position);
	} else {
		return choose_operator_on(std::forward<Options>(options)...);
	}
}

#endif
