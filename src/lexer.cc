#include "lexer.hpp"

#include "locale.hpp"
#include "common.hpp"
#include <locale>

Lexer::Lexer(std::unique_ptr<Source> src) :locale(Locale::get().locale()) {
    change_source( std::move(src) );
}

std::unique_ptr<Lexer> Lexer::from_source(std::unique_ptr<Source> source) {
    return std::make_unique<Lexer>( std::move(source) );
}

std::wstring Lexer::get_lines(std::size_t from, std::size_t to) {
    return source->get_lines(from, to);
}

Token Lexer::next() {
    if (!ch_opt) {
        ch_opt = source->next();
    }
    
    while (skip_space() || skip_comment());

    position = source->get_position();
    
    if (!ch_opt) {
        return make_token(TokenType::END_OF_FILE, position);
    }

    const auto ch = *ch_opt;
    
    if (std::isalpha(ch, locale) || ch == L'_') {
        return keyword_or_identifier();
    } else if (std::isdigit(ch, locale)) {
        return int_const();
    } else if (ch == L'"') {
        return string_const();
    } else if (operator_chars.find(ch) != operator_chars.end()){
        return operator_lexem();
    } else {
        report_error(position, L"Unrecognised character", *ch_opt);
    }
}

std::unique_ptr<Source> Lexer::change_source(std::unique_ptr<Source> src) noexcept {
    std::unique_ptr<Source> ret = std::move(source);
    source = std::move(src);
    return ret;
}


Token Lexer::keyword_or_identifier() {
    std::wstring str;
    wchar_t ch = *ch_opt;

    while (std::isalpha(ch, locale) || std::isdigit(ch, locale) || ch == L'_') {
        str += ch;
        ch_opt = source->next();
        if (!ch_opt) {
            break;
        }
        ch = *ch_opt;
    }

    if (keywords.find(str) != keywords.end()) {
        return make_token(keywords.at(str), position);
    } else {
        return make_token(TokenType::IDENTIFIER, position, str);
    }
}

Token Lexer::operator_lexem() {
    wchar_t ch = *ch_opt;
    ch_opt = source->next();
    switch (ch) {
        case L'~':  return make_token(TokenType::BIT_NEG, position);  
        case L'!':  return choose_operator_on(L'=', TokenType::NOT_EQUAL, TokenType::BOOLEAN_NEG);
        case L'%':  return make_token(TokenType::MODULO, position);
        case L'^':  return make_token(TokenType::XOR, position);
        case L'&':  return choose_operator_on(L'&', TokenType::BOOLEAN_AND, TokenType::AMPERSAND);
        case L'|':  return choose_operator_on(L'|', TokenType::BOOLEAN_OR, TokenType::BIT_OR);
        case L'*':  return make_token(TokenType::STAR, position);
        case L'(':  return make_token(TokenType::L_PAREN, position);
        case L')':  return make_token(TokenType::R_PAREN, position);
        case L'[':  return make_token(TokenType::LI_PAREN, position);
        case L']':  return make_token(TokenType::RI_PAREN, position);
        case L'{':  return make_token(TokenType::LS_PAREN, position);
        case L'}':  return make_token(TokenType::RS_PAREN, position);
        case L'<':  return choose_operator_on(L'<', TokenType::SHIFT_LEFT, L'=', TokenType::LESS_EQUAL, TokenType::LESS);
        case L'>':  return choose_operator_on(L'>', TokenType::SHIFT_RIGHT, L'=', TokenType::GREATER_EQUAL, TokenType::GREATER);
        case L'=':  return choose_operator_on(L'=', TokenType::EQUAL, TokenType::ASSIGN);
        case L'+':  return make_token(TokenType::PLUS, position);
        case L'-':  return choose_operator_on(L'>', TokenType::TYPE_DECL, TokenType::MINUS);
        case L':':  return make_token(TokenType::COLON, position);
        case L';':  return make_token(TokenType::SEMICOLON, position);
        case L',':  return make_token(TokenType::COMMA, position);
        case L'/':  return make_token(TokenType::DIVIDE, position);
    }
    if (ch == L'.' && ch_opt && *ch_opt == L'.') {
        ch_opt = source->next();
        return make_token(TokenType::RANGE_SEP, position);
    }
    report_error(position, L"Error operator undefined", *ch_opt);
}

Token Lexer::choose_operator_on(TokenType on_mismatch) {
    return make_token(on_mismatch, position);
}

bool Lexer::skip_space() {
    if (!std::isspace(*ch_opt, locale)) {
        return false;
    }    
    while (ch_opt && std::isspace(*ch_opt, locale)) {
        ch_opt = source->next();
    }
    return true;
}

bool Lexer::skip_comment() {
    if (*ch_opt != L'#') {
        return false;
    }
    while (ch_opt && *ch_opt != L'\n') {
        ch_opt = source->next();
    }
    return true;
}

Token Lexer::int_const() {
    std::wstring str;
    wchar_t ch = *ch_opt;

    while (std::isdigit(ch, locale)) {
        str += ch;
        ch_opt = source->next();
        if (!ch_opt) {
            break;
        }
        ch = *ch_opt;
    }

    try {
        return make_token(TokenType::INTCONST, position, std::stoi(str));
    } catch (std::invalid_argument) {
        report_error(position, L"Cannot convert this literal to int", str);
    } catch (std::out_of_range) {
        report_error(position, L"Number is to big to be albe to fit in an int", str);    
    }
}

Token Lexer::string_const() {
    std::wstring str;
    wchar_t ch;
    bool escaped = false;

    ch_opt = source->next();
    while (ch_opt) {
        ch = *ch_opt;

        if (escaped) {
            escaped = false;
            str += escape_char(ch);
        } else {
            if (ch == L'"') {
                ch_opt = source->next();
                return make_token(TokenType::STRINGCONST, position, str);
            } else if (ch == L'\\') {
                escaped = true;
            } else {
                str += ch;
            }
        }
        ch_opt = source->next();
    }
    report_error(position, L"Error reached end of file while collecting string", str);
}

wchar_t Lexer::escape_char(wchar_t ch) {
    switch (ch) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 'a': return '\a';
        case 'b': return '\b';
        case 't': return '\t';

        default: return ch;
    }
}

std::wstring Lexer::source_between(const Position& start, const Position& end) {
    return source->input_between(start, end);
}


void Lexer::report_error(const Position& error_position, const std::wstring& error_msg, wchar_t bad_char) {
    throw LexerException{
        concat(
            L"Error line ", std::to_wstring(error_position.line_number), L" in `\033[31;1;4m", bad_char, L"\033[0m`\n",
            error_msg
        )
    };
}

void Lexer::report_error(const Position& error_position, const std::wstring& error_msg, const std::wstring& bad_lexem) {
    throw LexerException{
        concat(
            L"Error line ", std::to_wstring(error_position.line_number), L" in `\033[31;1;4m", bad_lexem, L"\033[0m`\n",
            error_msg
        )
    };
}

const std::unordered_map<std::wstring, TokenType> Lexer::keywords = {
    { L"fn", TokenType::KW_FN },
    { L"for", TokenType::KW_FOR },
    { L"while", TokenType::KW_WHILE },
    { L"if", TokenType::KW_IF },
    { L"elif", TokenType::KW_ELIF },
    { L"else", TokenType::KW_ELSE },
    { L"return", TokenType::KW_RETURN },
    { L"let", TokenType::KW_LET },
    { L"in", TokenType::KW_IN },
    { L"extern", TokenType::KW_EXTERN }
};

const std::unordered_set<wchar_t> Lexer::operator_chars = {
    L'~',
    L'!',
    L'%',
    L'^',
    L'&',
    L'*',
    L'(',
    L')',
    L'[',
    L']',
    L'{',
    L'}',
    L'<',
    L'>',
    L'=',
    L'+',
    L'-',
    L'/',
    L':',
    L';',
    L',',
    L'|',
    L'.'
};
