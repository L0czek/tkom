#include "node.hpp"

#include "common.hpp"

BinaryOperator BinOp_from_token(const Token &token)
{
    switch (token.type) {
    case TokenType::PLUS:
        return BinaryOperator::Plus;
    case TokenType::MINUS:
        return BinaryOperator::Minus;
    case TokenType::STAR:
        return BinaryOperator::Multiply;
    case TokenType::DIVIDE:
        return BinaryOperator::Divide;
    case TokenType::MODULO:
        return BinaryOperator::Modulo;
    case TokenType::EQUAL:
        return BinaryOperator::Equal;
    case TokenType::NOT_EQUAL:
        return BinaryOperator::NotEqual;
    case TokenType::LESS:
        return BinaryOperator::Less;
    case TokenType::GREATER:
        return BinaryOperator::Greater;
    case TokenType::LESS_EQUAL:
        return BinaryOperator::LessEqual;
    case TokenType::GREATER_EQUAL:
        return BinaryOperator::GreaterEqual;
    case TokenType::AMPERSAND:
        return BinaryOperator::And;
    case TokenType::BIT_OR:
        return BinaryOperator::Or;
    case TokenType::XOR:
        return BinaryOperator::Xor;
    case TokenType::SHIFT_RIGHT:
        return BinaryOperator::ShiftRight;
    case TokenType::SHIFT_LEFT:
        return BinaryOperator::ShiftLeft;
    case TokenType::BOOLEAN_OR:
        return BinaryOperator::BooleanOr;
    case TokenType::BOOLEAN_AND:
        return BinaryOperator::BooleanAnd;
    default:
        throw std::runtime_error("Not a binary operator");
    }
}

UnaryOperator UnOp_from_token(const Token &token)
{
    switch (token.type) {
    case TokenType::MINUS:
        return UnaryOperator::Minus;
    case TokenType::STAR:
        return UnaryOperator::Deref;
    case TokenType::AMPERSAND:
        return UnaryOperator::Addrof;
    case TokenType::BIT_NEG:
        return UnaryOperator::Neg;
    case TokenType::BOOLEAN_NEG:
        return UnaryOperator::BooleanNeg;
    default:
        throw std::runtime_error("Not a unary operator");
    }
}
