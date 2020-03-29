#include "parser.hpp"

std::unique_ptr<Lexer> Parser::attach_lexer(std::unique_ptr<Lexer> lex) noexcept {
	auto tmp = std::move(lexer);
	lexer = std::move(lex);
	return tmp;
}

void Parser::advance() noexcept {
	token = lexer->next();
}

BuiltinType Parser::get_builtin_type(const std::wstring& wstr) const {
	if (wstr == L"int") {
		return BuiltinType::Int;
	} else if (wstr == L"string") {
		return BuiltinType::String;
	} else {
		throw ParserException{L"invalid type"};
	}
}

std::unique_ptr<Program> Parser::parse_Program() {
	advance();
	
	std::list<std::unique_ptr<FunctionDecl>> functions;
	std::list<std::unique_ptr<VariableDecl>> global_vars;

	while (!is_eof(token)) {
		expect(L"Expected function definition or global variable definition", TokenType::KW_FN, TokenType::KW_LET);

		if (token.type == TokenType::KW_FN) {
			functions.push_back(parse_FunctionDecl());
		} else {
			global_vars.push_back(parse_VariableDecl());
		}
	}

	return make<Program>( std::move(global_vars), std::move(functions) );
}

std::unique_ptr<FunctionDecl> Parser::parse_FunctionDecl() {
	advance();

	expect(L"Expected function name.", TokenType::IDENTIFIER);
	std::wstring name = *get_string(token);
	advance();
	expect(L"Expected `(` ", TokenType::L_PAREN);
	advance();
	std::list<std::pair<std::wstring, BuiltinType>> arguments = parse_DeclArgumentList();
	expect(L"Expected `)` ", TokenType::R_PAREN);
	advance();
	expect(L"Expected `->` to define return type.", TokenType::TYPE_DECL);
	advance();

	auto type = parse_VarType();
	auto block = parse_Block();

	return make<FunctionDecl>(std::move(name), type, std::move(arguments), std::move(block));
}

std::unique_ptr<VariableDecl> Parser::parse_VariableDecl() {
	VariableDecl::VarDeclList list;

	list.push_back(parse_SingleVarDecl());

	while (is_one_of(token, TokenType::COMMA)) {
		advance();
		list.push_back(parse_SingleVarDecl());
	}
	
	expect(L"Expected `:` to define varible type.", TokenType::COLON);
	advance();
	auto type = parse_VarType();
	
	expect(L"Expected `;` at the end of instruction.", TokenType::SEMICOLON);
	advance();

	for (auto &i : list) {
		i.type = type;
	}
	
	return make<VariableDecl>(std::move(list));
}

BuiltinType Parser::parse_VarType() {
	expect(L"Expected variable type", TokenType::IDENTIFIER);
	BuiltinType type = get_builtin_type(*get_string(token));
	advance();
	return type;
}

VariableDecl::SingleVarDecl Parser::parse_SingleVarDecl() {
	VariableDecl::SingleVarDecl var_decl;
	advance();
	expect(L"Expected variable name", TokenType::IDENTIFIER);
	var_decl.name = *get_string(token);
	
	advance();
	expect(L"Expected initial value assignment or next variable definition or type declaration", 
		TokenType::ASSIGN, TokenType::COMMA, TokenType::COLON);

	if (is_one_of(token, TokenType::COMMA, TokenType::COLON)) {
		return var_decl;
	} else {
		advance();
		var_decl.initial_value = parse_ArithmeticalExpr();
	}

	return var_decl;
}

std::list<std::pair<std::wstring, BuiltinType>> Parser::parse_DeclArgumentList() {
	std::list<std::pair<std::wstring, BuiltinType>> list;

	while (is_one_of(token, TokenType::IDENTIFIER)) {
		std::wstring name = *get_string(token);
		advance();
		expect(L"Expected `:` to define varible type.", TokenType::COLON);
		advance();
		auto type = parse_VarType();
		list.push_back(std::make_pair(std::move(name), type));
		if (!is_one_of(token, TokenType::COMMA)) {
			break;
		} else {
			advance(); // next argument
		}
	}

	return list;
}

std::unique_ptr<Expression> Parser::parse_ConditionalExpression(std::unique_ptr<Expression> prev_lhs) {
	auto lhs = prev_lhs && is_boolean_binary_op(token) ? std::move(prev_lhs) : parse_UnaryLogicalExpr(std::move(prev_lhs));

	if (is_one_of(token, TokenType::BOOLEAN_AND, TokenType::BOOLEAN_OR)) {
		auto op = BinOp_from_token(token);
		advance();
		auto rhs = parse_ConditionalExpression();
		return make<BinaryExpression>(op, std::move(lhs), std::move(rhs));
	} else {
		return lhs;
	}
}

std::unique_ptr<Expression> Parser::parse_UnaryLogicalExpr(std::unique_ptr<Expression> prev_lhs) {
	if (prev_lhs) {
		return parse_LogicalExpr(std::move(prev_lhs));
	} else {
		if (is_one_of(token, TokenType::BOOLEAN_NEG)) {
			advance();
			return make<UnaryExpression>(UnaryOperator::BooleanNeg, parse_LogicalExpr());
		} else {
			return parse_LogicalExpr();
		}
	}
}

std::unique_ptr<Expression> Parser::parse_LogicalExpr(std::unique_ptr<Expression> prev_lhs) {
	auto lhs = prev_lhs && is_compare_op(token) ? std::move(prev_lhs) : parse_ArithmeticalExpr(std::move(prev_lhs));

	if (is_compare_op(token)) {
		auto op = BinOp_from_token(token);
		advance();
		return make<BinaryExpression>(op, std::move(lhs), parse_LogicalExpr());
	} else {
		return lhs;
	}
}

std::unique_ptr<Expression> Parser::parse_ArithmeticalExpr(std::unique_ptr<Expression> prev_lhs) {
	auto lhs = prev_lhs && is_bitwise_op(token) ? std::move(prev_lhs) : parse_AdditiveExpr(std::move(prev_lhs));

	if (is_bitwise_op(token)) {
		auto op = BinOp_from_token(token);
		advance();
		return make<BinaryExpression>(op, std::move(lhs), parse_ArithmeticalExpr());
	} else {
		return lhs;
	}
}

std::unique_ptr<Expression> Parser::parse_AdditiveExpr(std::unique_ptr<Expression> prev_lhs) {
	auto lhs = prev_lhs && is_additive_op(token) ? std::move(prev_lhs) : parse_MultiplicativeExpr(std::move(prev_lhs));

	if (is_additive_op(token)) {
		auto op = BinOp_from_token(token);
		advance();
		return make<BinaryExpression>(op, std::move(lhs), parse_AdditiveExpr());
	} else {
		return lhs;
	}
}

std::unique_ptr<Expression> Parser::parse_MultiplicativeExpr(std::unique_ptr<Expression> prev_lhs) {
	auto lhs = prev_lhs && is_multiplicative_op(token) ? std::move(prev_lhs) : parse_UnaryExpression(std::move(prev_lhs));

	if (is_multiplicative_op(token)) {
		auto op = BinOp_from_token(token);
		advance();
		return make<BinaryExpression>(op, std::move(lhs), parse_MultiplicativeExpr());
	} else {
		return lhs;
	}
}

std::unique_ptr<Expression> Parser::parse_UnaryExpression(std::unique_ptr<Expression> prev_lhs) {
	if (prev_lhs) {
		return prev_lhs;
	}

	while (is_unary_op(token)) {
		stack.push(UnOp_from_token(token));
		advance();
	}

	auto node = parse_Factor();

	if (token.type == TokenType::LI_PAREN) {
		node = parse_IndexExpression(std::move(node));
	}

	while (!stack.empty()) {
		node = make<UnaryExpression>(stack.top(), std::move(node));
		stack.pop();
	}

	return node;
}

std::unique_ptr<Expression> Parser::parse_Factor() {
	if (token.type == TokenType::L_PAREN) {
		advance();
		auto node = parse_ConditionalExpression();
		expect(L"Expected `)`", TokenType::R_PAREN);
	}

	if (is_one_of(token, TokenType::INTCONST, TokenType::STRINGCONST)) {
		std::unique_ptr<Expression> expr;
		switch (token.type) {
			case TokenType::INTCONST:     expr = make<IntConst>(*get_int(token)); break;
			case TokenType::STRINGCONST:  expr = make<StringConst>(*get_string(token)); break;
		}
		advance();
		return expr;
	}

	if (token.type == TokenType::IDENTIFIER) {
		std::wstring name = *get_string(token);
		advance();
		if (token.type == TokenType::L_PAREN) {
			return parse_FunctionCall(std::move(name));
		} else {
			return make<VariableRef>(std::move(name));
		}
	}

	throw ParserException{L"crazy shit is going on"};
}

std::unique_ptr<FunctionCall> Parser::parse_FunctionCall(std::wstring name) {
	advance();
	std::list<std::unique_ptr<Expression>> arguments = parse_CallArgumentList();

	expect(L"Expected `)` after arguments list", TokenType::R_PAREN);
	advance();

	return make<FunctionCall>(std::move(name), std::move(arguments));
}

std::list<std::unique_ptr<Expression>> Parser::parse_CallArgumentList() {
	if (token.type == TokenType::R_PAREN) {
		return {};
	}

	std::list<std::unique_ptr<Expression>> arguments;
	arguments.push_back(parse_ArithmeticalExpr());

	while (token.type == TokenType::COMMA) {
		advance();
		arguments.push_back(parse_ArithmeticalExpr());
	}

	return arguments;
}

std::unique_ptr<Expression> Parser::parse_MutableExpression() {
	if (token.type == TokenType::STAR) {
		advance();
		auto node = parse_PointerExpr();
		if (!node) {
			stack.push(UnOp_from_token(token));
			return node;
		} else {
			return make<UnaryExpression>(UnaryOperator::Deref, std::move(node));
		}
	}

	std::unique_ptr<Expression> node;
	if (token.type == TokenType::IDENTIFIER) {
		std::wstring name = *get_string(token);
		advance();
		if (token.type == TokenType::L_PAREN) {
			node = parse_FunctionCall(std::move(name));
		} else {
			node = make<VariableRef>(std::move(name));
		}
		if (token.type == TokenType::LI_PAREN) {
			node = parse_IndexExpression(std::move(node));
		}
	}
	return node;
}

std::unique_ptr<Expression> Parser::parse_IndexExpression(std::unique_ptr<Expression> ptr) {
	advance();
	auto index = parse_ArithmeticalExpr();
	expect(L"Expected `]`", TokenType::RI_PAREN);
	advance();
	return make<IndexExpression>(std::move(ptr), std::move(index));
}

std::unique_ptr<Expression> Parser::parse_PointerExpr() {
	if (token.type == TokenType::AMPERSAND) {
		advance();
		expect(L"Expected variable name", TokenType::IDENTIFIER);
		std::wstring name = *get_string(token);
		advance();
		return make<UnaryExpression>(UnaryOperator::Addrof, make<VariableRef>(std::move(name)));
	}


	if (token.type == TokenType::IDENTIFIER) {
		std::wstring name = *get_string(token);
		advance();
		if (token.type == TokenType::L_PAREN) {
			return parse_FunctionCall(std::move(name));
		} else {
			return make<VariableRef>(std::move(name));
		}
	}
	
	if (token.type == TokenType::L_PAREN) {
		advance();
		auto node = parse_ArithmeticalExpr();
		expect(L"Expected `)`", TokenType::R_PAREN);
		advance();
		return node;
	}
}

std::unique_ptr<Statement> Parser::parse_AssignStatement() {
	if (is_one_of(token, TokenType::BIT_NEG, TokenType::BOOLEAN_NEG, TokenType::AMPERSAND, 
						 TokenType::INTCONST, TokenType::STRINGCONST)) {
		return parse_ExpressionStatement();
	}

	auto node = parse_MutableExpression();
	if (!node || token.type != TokenType::ASSIGN) {
		return parse_ExpressionStatement(std::move(node));
	}
	std::list<std::unique_ptr<Expression>> parts;
	parts.push_back(std::move(node));

	while (token.type == TokenType::ASSIGN) {
		advance();
		node = parse_MutableExpression();
		if (!node) {
			break;
		}
		parts.push_back(std::move(node));
	}

	if (token.type != TokenType::SEMICOLON) {
		parts.push_back(parse_ArithmeticalExpr());
	}
	
	expect(L"Expected `;` after statement", TokenType::SEMICOLON);
	advance();
	return make<AssignmentStatement>(std::move(parts));
}

std::unique_ptr<ExpressionStatement> Parser::parse_ExpressionStatement(std::unique_ptr<Expression> prev_lhs) {
	auto node = parse_ConditionalExpression(std::move(prev_lhs));
	expect(L"Expected `;` after expression", TokenType::SEMICOLON);
	advance();
	return make<ExpressionStatement>(std::move(node));
}

std::unique_ptr<Statement> Parser::parse_Statement() {
	switch (token.type) {
		case TokenType::KW_LET:    return parse_VariableDecl();
		case TokenType::KW_IF:     return parse_IfStatement();
		case TokenType::KW_FOR:    return parse_ForStatement();
		case TokenType::KW_WHILE:  return parse_WhileStatement();
		case TokenType::KW_RETURN: return parse_ReturnSatetemnt();
		default:
			return parse_AssignStatement();
	}
}

std::unique_ptr<IfStatement> Parser::parse_IfStatement() {
	advance();

	std::list<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block>>> blocks;
	std::optional<std::unique_ptr<Block>> else_statement;

	auto condition = parse_ConditionalExpression();
	auto block = parse_Block();

	blocks.push_back(std::make_pair(std::move(condition), std::move(block)));

	while (token.type == TokenType::KW_ELIF) {
		advance();
		auto condition = parse_ConditionalExpression();
		auto block = parse_Block();
	
		blocks.push_back(std::make_pair(std::move(condition), std::move(block)));
	}

	if (token.type == TokenType::KW_ELSE) {
		advance();
		else_statement = parse_Block();
	}

	return make<IfStatement>(std::move(blocks), std::move(else_statement));
}

std::unique_ptr<ForStatement> Parser::parse_ForStatement() {
	advance();
	expect(L"Expected varible name", TokenType::IDENTIFIER);
	std::wstring name = *get_string(token);
	advance();
	expect(L"Expected `in` keyword", TokenType::KW_IN);
	advance();

	auto start = parse_ArithmeticalExpr();
	expect(L"Expected `..` range separator", TokenType::RANGE_SEP);
	advance();
	auto end = parse_ArithmeticalExpr();
	std::optional<std::unique_ptr<Expression>> increase;

	if (token.type == TokenType::RANGE_SEP) {
		advance();
		increase = parse_ArithmeticalExpr();
	}

	auto block = parse_Block();

	return make<ForStatement>(std::move(name), std::move(start), std::move(end), std::move(increase), std::move(block));
}

std::unique_ptr<WhileStatement> Parser::parse_WhileStatement() {
	advance();

	auto condition = parse_ConditionalExpression();
	auto block = parse_Block();

	return make<WhileStatement>(std::move(condition), std::move(block));
}

std::unique_ptr<ReturnStatement> Parser::parse_ReturnSatetemnt() {
	advance();

	auto value = parse_ArithmeticalExpr();

	expect(L"Expected `;` at the end of return statement", TokenType::SEMICOLON);
	advance();
	return make<ReturnStatement>(std::move(value));
}

std::unique_ptr<Block> Parser::parse_Block() {
	expect(L"Expected `{` at the beginning of block", TokenType::LS_PAREN);
	advance();

	std::list<std::unique_ptr<Statement>> block;

	while (token.type != TokenType::RS_PAREN) {
		block.push_back(parse_Statement());
	}
	advance();

	return make<Block>(std::move(block));
}

void Parser::report_error(const Position& start, const Position& end, const std::wstring& error_msg) {
	throw ParserException{
		concat(L"line :", std::to_wstring(start.line_number), L" in `", lexer->source_between(start, end), L"`\n", error_msg)
	};
}

