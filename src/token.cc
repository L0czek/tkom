#include "token.hpp"

#include "common.hpp"

bool is_expr_operator(const Token& token) {
    return static_cast<std::size_t>(token.type) & 0x100;
}

bool is_literal(const Token& token) {
    return static_cast<std::size_t>(token.type) <= 0xff;
}

bool is_syntax_separator(const Token& token) {
    return static_cast<std::size_t>(token.type) & 0x200;
}

bool is_valid(const Token& token) {
    return token.type != TokenType::INVALID;
}


bool is_eof(const Token& token) {
    return token.type == TokenType::END_OF_FILE; 
}

bool is_kewyord(const Token& token) {
    return static_cast<std::size_t>(token.type) & 0x800;
}

std::optional<int> get_int(const Token& token) {
    try {
        return std::get<int>(token.value);
    } catch (std::bad_variant_access) {
        return {};
    }
}

std::optional<std::wstring> get_string(const Token& token) {
    try {
        return std::get<std::wstring>(token.value);
    } catch (std::bad_variant_access) {
        return {};    
    }
}

Token make_token(TokenType type, const Position& position) {
    return Token{type, position};
}

Token make_token(TokenType type, const Position& position, int value) {
    return Token{type, position, value};
}

Token make_token(TokenType type, const Position& position, std::wstring value) {
    return Token{type, position, value};
}


std::wstring repr(const Token& token) {
    switch (token.type) {
        case TokenType::IDENTIFIER:     return concat(to_wstring(token.position), L"TOKEN(IDENTIFIER, ", *get_string(token), L")");       break;
        case TokenType::INTCONST:       return concat(to_wstring(token.position), L"TOKEN(INTCONST, ", std::to_wstring(*get_int(token)), L")");         break;
        case TokenType::STRINGCONST:    return concat(to_wstring(token.position), L"TOKEN(STRINGCONST, ", *get_string(token), L")");      break;
        case TokenType::PLUS:           return concat(to_wstring(token.position), L"TOKEN(PLUS)");             break;
        case TokenType::MINUS:          return concat(to_wstring(token.position), L"TOKEN(MINUS)");            break;
        case TokenType::STAR:           return concat(to_wstring(token.position), L"TOKEN(STAR)");             break;
        case TokenType::DIVIDE:         return concat(to_wstring(token.position), L"TOKEN(DIVIDE)");           break;
        case TokenType::MODULO:         return concat(to_wstring(token.position), L"TOKEN(MODULO)");           break;
        case TokenType::ASSIGN:         return concat(to_wstring(token.position), L"TOKEN(ASSIGN)");           break;
        case TokenType::EQUAL:          return concat(to_wstring(token.position), L"TOKEN(EQUAL)");            break;
        case TokenType::NOT_EQUAL:      return concat(to_wstring(token.position), L"TOKEN(NOT_EQUAL)");        break;
        case TokenType::LESS:           return concat(to_wstring(token.position), L"TOKEN(LESS)");             break;
        case TokenType::GREATER:        return concat(to_wstring(token.position), L"TOKEN(GREATER)");          break;
        case TokenType::LESS_EQUAL:     return concat(to_wstring(token.position), L"TOKEN(LESS_EQUAL)");       break;
        case TokenType::GREATER_EQUAL:  return concat(to_wstring(token.position), L"TOKEN(GREATER_EQUAL)");    break;
        case TokenType::AMPERSAND:      return concat(to_wstring(token.position), L"TOKEN(AMPERSAND)");        break;
        case TokenType::BIT_OR:         return concat(to_wstring(token.position), L"TOKEN(BIT_OR)");           break;
        case TokenType::XOR:            return concat(to_wstring(token.position), L"TOKEN(XOR)");              break;
        case TokenType::SHIFT_RIGHT:    return concat(to_wstring(token.position), L"TOKEN(SHIFT_RIGHT)");      break;
        case TokenType::SHIFT_LEFT:     return concat(to_wstring(token.position), L"TOKEN(SHIFT_LEFT)");       break;
        case TokenType::BIT_NEG:        return concat(to_wstring(token.position), L"TOKEN(BIT_NEG)");          break;
        case TokenType::BOOLEAN_OR:     return concat(to_wstring(token.position), L"TOKEN(BOOLEAN_OR)");       break;
        case TokenType::BOOLEAN_AND:    return concat(to_wstring(token.position), L"TOKEN(BOOLEAN_AND)");      break;
        case TokenType::BOOLEAN_NEG:    return concat(to_wstring(token.position), L"TOKEN(BOOLEAN_NEG)");      break;
        case TokenType::L_PAREN:        return concat(to_wstring(token.position), L"TOKEN(L_PAREN)");          break;
        case TokenType::R_PAREN:        return concat(to_wstring(token.position), L"TOKEN(R_PAREN)");          break;
        case TokenType::LI_PAREN:       return concat(to_wstring(token.position), L"TOKEN(LI_PAREN)");         break;
        case TokenType::RI_PAREN:       return concat(to_wstring(token.position), L"TOKEN(RI_PAREN)");         break;
        case TokenType::LS_PAREN:       return concat(to_wstring(token.position), L"TOKEN(LS_PAREN)");         break;
        case TokenType::RS_PAREN:       return concat(to_wstring(token.position), L"TOKEN(RS_PAREN)");         break;
        case TokenType::COLON:          return concat(to_wstring(token.position), L"TOKEN(COLON)");            break;
        case TokenType::COMMA:          return concat(to_wstring(token.position), L"TOKEN(COMMA)");            break;
        case TokenType::SEMICOLON:      return concat(to_wstring(token.position), L"TOKEN(SEMICOLON)");        break;
        case TokenType::TYPE_DECL:      return concat(to_wstring(token.position), L"TOKEN(TYPE_DECL)");        break;
        case TokenType::END_OF_FILE:    return concat(to_wstring(token.position), L"TOKEN(END_OF_FILE)");      break;
        case TokenType::KW_FN:          return concat(to_wstring(token.position), L"TOKEN(KW_FN)");            break;
        case TokenType::KW_FOR:         return concat(to_wstring(token.position), L"TOKEN(KW_FOR)");           break;
        case TokenType::KW_WHILE:       return concat(to_wstring(token.position), L"TOKEN(KW_WHILE)");         break;
        case TokenType::KW_IF:          return concat(to_wstring(token.position), L"TOKEN(KW_IF)");            break;
        case TokenType::KW_ELSE:        return concat(to_wstring(token.position), L"TOKEN(KW_ELSE)");          break;
        case TokenType::KW_ELIF:        return concat(to_wstring(token.position), L"TOKEN(KW_ELIF)");          break;
        case TokenType::KW_RETURN:      return concat(to_wstring(token.position), L"TOKEN(KW_RETURN)");        break;
        case TokenType::KW_LET:         return concat(to_wstring(token.position), L"TOKEN(KW_LET)");           break;
        case TokenType::INVALID:        return concat(to_wstring(token.position), L"TOKEN(INVALID)");          break;
        default: return L"No such token";
    }
}
