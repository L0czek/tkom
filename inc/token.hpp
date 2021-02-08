#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__

#include <string>
#include <cstdint>
#include <variant>
#include <optional>
#include <unordered_map>

#include "source.hpp"

enum class TokenType {
	IDENTIFIER = 0,
	KEYWORD,
	INTCONST,
	STRINGCONST,

	PLUS = 0x100, // '+'
	MINUS, // '-'
	STAR, // '*'
	DIVIDE, // '/'
	MODULO, // '%'

	ASSIGN, // '='

	EQUAL, // '=='
	NOT_EQUAL, // '!='
	LESS, // '<'
	GREATER, // '>'
	LESS_EQUAL, // '<='
	GREATER_EQUAL, // '>='

	AMPERSAND, // '&'
	BIT_OR, // '|'
	XOR, // '^'
	SHIFT_RIGHT, // '>>'
	SHIFT_LEFT, // '<<'
	BIT_NEG, // '~'

	BOOLEAN_OR, // '||'
	BOOLEAN_AND, // '&&'
	BOOLEAN_NEG, // '!'

	L_PAREN, // '('
	R_PAREN, // ')'

	LI_PAREN, // '['
	RI_PAREN, // ']'

	LS_PAREN = 0x200, // '{'
	RS_PAREN, // '}'

	COLON, // ':'
	COMMA, // ','
	SEMICOLON, // ';'
	RANGE_SEP, // '..'

	TYPE_DECL, // '->'

	END_OF_FILE = 0x400,

	KW_FN = 0x800,
	KW_FOR,
	KW_IN,
	KW_WHILE,
	KW_IF,
	KW_ELSE,
	KW_ELIF,
	KW_RETURN,
	KW_LET,
	KW_EXTERN,

	INVALID = 0x1'000
};

struct Token {
	TokenType type;
	Position position;
	std::variant<int, // INT_CONST
		     std::wstring // STRING_CONST
		     >
		value;
};

bool is_expr_operator(const Token &token);
bool is_literal(const Token &token);
bool is_syntax_separator(const Token &token);
bool is_valid(const Token &token);
bool is_eof(const Token &token);
bool is_kewyord(const Token &token);

bool is_compare_op(const Token &token);
bool is_boolean_binary_op(const Token &token);
bool is_bitwise_op(const Token &token);
bool is_additive_op(const Token &token);
bool is_multiplicative_op(const Token &token);
bool is_unary_op(const Token &token);

template <typename... Types> bool is_one_of(const Token &token, TokenType type, Types &&... types)
{
	return token.type == type ? true : is_one_of(token, std::forward<Types>(types)...);
}

inline bool is_one_of(const Token &)
{
	return false;
}

std::optional<int> get_int(const Token &token);
std::optional<std::wstring> get_string(const Token &token);

Token make_token(TokenType type, const Position &position);
Token make_token(TokenType type, const Position &position, int value);
Token make_token(TokenType type, const Position &position, std::wstring value);

std::wstring position_in_file(const Token &token);
std::wstring position_in_file(const Position &position);

std::wstring repr(const Token &token);
std::wstring repr(TokenType type);

#endif
