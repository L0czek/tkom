#include "node.hpp"

#include "common.hpp"

BinaryOperator BinOp_from_token(const Token& token) {
	switch (token.type) {
		case TokenType::PLUS: 			return BinaryOperator::Plus;									
    	case TokenType::MINUS: 			return BinaryOperator::Minus;									
    	case TokenType::STAR: 			return BinaryOperator::Multiply;									
    	case TokenType::DIVIDE: 		return BinaryOperator::Divide;										
    	case TokenType::MODULO: 		return BinaryOperator::Modulo;																				
    	case TokenType::EQUAL: 			return BinaryOperator::Equal;									
    	case TokenType::NOT_EQUAL: 		return BinaryOperator::NotEqual;										
    	case TokenType::LESS: 			return BinaryOperator::Less;									
    	case TokenType::GREATER: 		return BinaryOperator::Greater;										
    	case TokenType::LESS_EQUAL: 	return BinaryOperator::LessEqual;											
    	case TokenType::GREATER_EQUAL: 	return BinaryOperator::GreaterEqual;											
    	case TokenType::AMPERSAND: 		return BinaryOperator::And;										
    	case TokenType::BIT_OR: 		return BinaryOperator::Or;										
    	case TokenType::XOR: 			return BinaryOperator::Xor;									
    	case TokenType::SHIFT_RIGHT: 	return BinaryOperator::ShiftRight;											
    	case TokenType::SHIFT_LEFT: 	return BinaryOperator::ShiftLeft;											
    	case TokenType::BOOLEAN_OR: 	return BinaryOperator::BooleanOr;											
    	case TokenType::BOOLEAN_AND: 	return BinaryOperator::BooleanAnd;											
	}
}


UnaryOperator UnOp_from_token(const Token& token) {
	switch (token.type) {
		case TokenType::MINUS:       return UnaryOperator::Minus;
		case TokenType::STAR:        return UnaryOperator::Deref;
		case TokenType::AMPERSAND:   return UnaryOperator::Addrof;
		case TokenType::BIT_NEG:     return UnaryOperator::Neg;
		case TokenType::BOOLEAN_NEG: return UnaryOperator::BooleanNeg;
	}
}


std::wstring UnaryExpression::repr() const noexcept {
	return concat(L"UnaryExpression = [ op =  ; rhs = ", rhs->repr(), L" ]");
}

std::wstring BinaryExpression::repr() const noexcept {
	return concat(L"BinaryExpression = [ op =  ; rhs = ", rhs->repr(), L" ; lhs = ", lhs->repr(), L" ]");
}

std::wstring VariableRef::repr() const noexcept {
	return concat(L"VariableRef = [ id = ", var_name, L" ]");
}

std::wstring FunctionCall::repr() const noexcept {
	return concat(L"FunctionCall = [ name = ", func_name, L" ; args = [ ]");
}

std::wstring Block::repr() const noexcept {
	return concat(L"Block = [  ]");
}

std::wstring FunctionDecl::repr() const noexcept {
	return concat(L"FunctionDecl = [ name = ", func_name, L" ]");
}

std::wstring VariableDecl::repr() const noexcept {
	return concat(L"VariableDecl = ");
}

std::wstring AssignmentStatement::repr() const noexcept {
	return concat(L"AssignmentStatement = ");
}

std::wstring ReturnStatement::repr() const noexcept {
	return concat(L"ReturnStatement = ");
}

std::wstring ExpressionStatement::repr() const noexcept {
	return concat(L"ExpressionStatement = ");
}

std::wstring IfStatement::repr() const noexcept {
	return concat(L"IfStatement = ");
}

std::wstring ForStatement::repr() const noexcept {
	return concat(L"ForStatement = ");
}

std::wstring WhileStatement::repr() const noexcept {
	return concat(L"WhileStatement = ");
}

std::wstring Program::repr() const noexcept {
	return concat(L"Program = ");
}

std::wstring repr(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Plus: return L"Plus";
        case BinaryOperator::Minus: return L"Minus";


        case BinaryOperator::Multiply: return L"Multiply";
        case BinaryOperator::Divide: return L"Divide";
        case BinaryOperator::Modulo: return L"Modulo";


        case BinaryOperator::And: return L"And";
        case BinaryOperator::Xor: return L"Xor";
        case BinaryOperator::Or: return L"Or";
        case BinaryOperator::ShiftLeft: return L"ShiftLeft";
        case BinaryOperator::ShiftRight: return L"ShiftRight";


        case BinaryOperator::Less: return L"Less";
        case BinaryOperator::Greater: return L"Greater";
        case BinaryOperator::LessEqual: return L"LessEqual";
        case BinaryOperator::GreaterEqual: return L"GreaterEqual";
        case BinaryOperator::Equal: return L"Equal";
        case BinaryOperator::NotEqual: return L"NotEqual";


        case BinaryOperator::BooleanAnd: return L"BooleanAnd";
        case BinaryOperator::BooleanOr: return L"BooleanOr";
    }
}

std::wstring repr(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Neg: return L"Neg";
        case UnaryOperator::Addrof: return L"Addrof";
        case UnaryOperator::BooleanNeg: return L"BooleanNeg";
    }
}