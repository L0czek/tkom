#include "lexer.hpp"

#include "locale.hpp"
#include "common.hpp"
#include <locale>

Lexer::Lexer(std::unique_ptr<Source> src) :locale(Locale::get().locale()) {
    change_source( std::move(src) );
}

Token Lexer::next() {
    if (!ch_opt) {
        ch_opt = source->next();
    }
    skip_space();
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
        return operator_or_comment();
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

Token Lexer::operator_or_comment() {
    wchar_t ch = *ch_opt;
    Token token;

    if (ch == L'~') {
        token = make_token(TokenType::BIT_NEG, position);  
    } else if (ch == L'!') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'=') {
            token = make_token(TokenType::NOT_EQUAL, position);
        } else {
            return make_token(TokenType::BOOLEAN_NEG, position);
        }
    } else if (ch == L'%') {
        token = make_token(TokenType::MODULO, position);
    } else if (ch == L'^') {
        token = make_token(TokenType::XOR, position);
    } else if (ch == L'&') { 
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'&') {
            token = make_token(TokenType::BOOLEAN_AND, position);
        } else {
            return make_token(TokenType::AMPERSAND, position);
        }
    } else if (ch == L'|') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'|') {
            token = make_token(TokenType::BOOLEAN_OR, position);
        } else {
            return make_token(TokenType::BIT_OR, position);
        }
    } else if (ch == L'*') {
        token = make_token(TokenType::STAR, position);
    } else if (ch == L'(') {
        token = make_token(TokenType::L_PAREN, position);
    } else if (ch == L')') {
        token = make_token(TokenType::R_PAREN, position);
    } else if (ch == L'[') {
        token = make_token(TokenType::LI_PAREN, position);
    } else if (ch == L']') {
        token = make_token(TokenType::RI_PAREN, position);
    } else if (ch == L'{') {
        token = make_token(TokenType::LS_PAREN, position);
    } else if (ch == L'}') {
        token = make_token(TokenType::RS_PAREN, position);
    } else if (ch == L'<') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'<') {
            token = make_token(TokenType::SHIFT_LEFT, position); 
        } else if (ch_opt && *ch_opt == L'=') {
            token = make_token(TokenType::LESS_EQUAL, position);
        } else {
            return make_token(TokenType::LESS, position);
        }
    } else if (ch == L'>') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'>') {
            token = make_token(TokenType::SHIFT_RIGHT, position); 
        } else if (ch_opt && *ch_opt == L'=') {
            token = make_token(TokenType::GREATER_EQUAL, position);
        } else {
            return make_token(TokenType::GREATER, position);
        }
    } else if (ch == L'=') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'=') {
            token = make_token(TokenType::EQUAL, position);
        } else {
            return make_token(TokenType::ASSIGN, position);
        }
    } else if (ch == L'+') {
        token = make_token(TokenType::PLUS, position);
    } else if (ch == L'-') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'>') {
            token = make_token(TokenType::TYPE_DECL, position);
        } else {
            return make_token(TokenType::MINUS, position);
        }
    } else if (ch == L':') {
        token = make_token(TokenType::COLON, position);
    } else if (ch == L';') {
        token = make_token(TokenType::SEMICOLON, position);
    } else if (ch == L',') {
        token = make_token(TokenType::COMMA, position);
    } else if (ch == L'/') {
        ch_opt = source->next();
        if (ch_opt && *ch_opt == L'/') {
            while (ch_opt && ( skip_comment() || skip_space() ) ) {
                if (*ch_opt == L'/') {
                    ch_opt = source->next();
                    if (ch_opt && *ch_opt == L'/') {
                        continue;
                    } else {
                        return make_token(TokenType::DIVIDE, position);
                    }
                } else {
                    return next(); // will never reach here again in recursion beacuse we do not allow `/` in ch_opt
                }
            } 
        } else {
            return make_token(TokenType::DIVIDE, position);
        }
    } else {
        report_error(position, L"Error operator undefined", *ch_opt);
    }
    ch_opt = source->next();
    return token;
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
    if (*ch_opt != L'/') {
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

    if (!std::isspace(ch, locale) && operator_chars.find(ch) == operator_chars.end()) {
        report_error(position, L"Illegal character in number", str + ch);
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
            str += ch;
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
    { L"let", TokenType::KW_LET }
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
    L'|'
};
