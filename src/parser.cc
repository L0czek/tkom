#include "parser.hpp"
#include <iostream>
std::unique_ptr<Lexer> Parser::attach_lexer(std::unique_ptr<Lexer> lex) noexcept
{
    auto tmp = std::move(lexer);
    lexer = std::move(lex);
    advance();
    return tmp;
}

std::unique_ptr<Lexer> Parser::detach_lexer() noexcept
{
    return std::move(lexer);
}

void Parser::advance() noexcept
{
    token = lexer->next();
}

std::unique_ptr<Program> Parser::parse()
{
    return parse_Program();
}

BuiltinType Parser::get_builtin_type(const std::wstring &wstr) const
{
    if (wstr == L"int") {
        return BuiltinType::Int;
    } else if (wstr == L"string") {
        return BuiltinType::String;
    } else {
        report_invalid_type();
    }
}

std::unique_ptr<Program> Parser::parse_Program()
{
    std::list<std::unique_ptr<VariableDecl> > global_vars;
    std::list<std::unique_ptr<FunctionDecl> > functions;
    std::list<std::unique_ptr<ExternFunctionDecl> > externs;

    auto function = parse_FunctionDecl();
    auto variable = parse_VariableDecl();
    auto extern_func = parse_ExternFunctionDecl();

    while (function || variable || extern_func) {
        if (function) {
            functions.push_back(std::move(function));
        }
        if (variable) {
            global_vars.push_back(std::move(variable));
        }
        if (extern_func) {
            externs.push_back(std::move(extern_func));
        }
        function = parse_FunctionDecl();
        variable = parse_VariableDecl();
        extern_func = parse_ExternFunctionDecl();
    }

    expect(L"Expected function `fn` declaration or variable `let` definition token", TokenType::END_OF_FILE);

    return make<Program>(std::move(global_vars), std::move(functions), std::move(externs));
}

std::unique_ptr<ExternFunctionDecl> Parser::parse_ExternFunctionDecl()
{
    if (!is_one_of(token, TokenType::KW_EXTERN)) {
        return nullptr;
    }
    const Position position = token.position;
    advance();

    eat(L"Expected `fn` keyword", TokenType::KW_FN);
    expect(L"Expected function name", TokenType::IDENTIFIER);
    std::wstring name = *get_string(token);
    advance();

    eat(L"Expected openning paren `(`", TokenType::L_PAREN);
    auto parameters = parse_ParameterList();
    eat(L"Expected closing paren `)`", TokenType::R_PAREN);

    eat(L"Expected type declaration `->` token", TokenType::TYPE_DECL);
    auto type = parse_Type();
    eat(L"Expected `;` after extern function declaration", TokenType::SEMICOLON);

    return make<ExternFunctionDecl>(position, std::move(name), type, std::move(parameters));
}

std::unique_ptr<FunctionDecl> Parser::parse_FunctionDecl()
{
    if (!is_one_of(token, TokenType::KW_FN)) {
        return nullptr;
    }
    const Position position = token.position;
    advance();

    expect(L"Expected function name", TokenType::IDENTIFIER);
    std::wstring name = *get_string(token);
    advance();

    eat(L"Expected openning paren `(`", TokenType::L_PAREN);
    auto parameters = parse_ParameterList();
    eat(L"Expected closing paren `)`", TokenType::R_PAREN);

    eat(L"Expected type declaration `->` token", TokenType::TYPE_DECL);
    auto type = parse_Type();
    auto block = parse_Block();

    return make<FunctionDecl>(position, std::move(name), type, std::move(parameters), std::move(block));
}

std::unique_ptr<VariableDecl> Parser::parse_VariableDecl()
{
    if (!is_one_of(token, TokenType::KW_LET)) {
        return nullptr;
    }
    advance();

    VariableDecl::VarDeclList list;

    list.push_back(parse_SingleVarDecl());

    while (is_one_of(token, TokenType::COMMA)) {
        advance();
        list.push_back(parse_SingleVarDecl());
    }

    eat(L"Expected type declaration `:` token", TokenType::COLON);
    auto type = parse_Type();
    eat(L"Expected semicolon `;` at the end of statement", TokenType::SEMICOLON);

    for (auto &i : list) {
        i.type = type;
    }
    return make<VariableDecl>(std::move(list));
}

VariableDecl::SingleVarDecl Parser::parse_SingleVarDecl()
{
    VariableDecl::SingleVarDecl var;

    expect(L"Expected variable name", TokenType::IDENTIFIER);
    var.name = *get_string(token);
    var.pos = token.position;
    advance();

    if (is_one_of(token, TokenType::ASSIGN)) {
        advance();
        auto value = parse_ArithmeticalExpr();
        if (!value) {
            report_expected_expression();
        }
        var.initial_value = std::move(value);
    }

    return var;
}

BuiltinType Parser::parse_Type()
{
    expect(L"Expected type name", TokenType::IDENTIFIER);
    std::wstring name = *get_string(token);

    if (name == L"int") {
        advance();
        if (is_one_of(token, TokenType::STAR)) {
            advance();
            return BuiltinType::IntPointer;
        }
        return BuiltinType::Int;
    } else if (name == L"string") {
        advance();
        return BuiltinType::String;
    } else {
        report_invalid_type();
    }
}

std::list<FunctionDecl::Parameter> Parser::parse_ParameterList()
{
    std::list<FunctionDecl::Parameter> list;
    auto param = parse_SingleParameter();
    if (!param) {
        return {};
    } else {
        list.push_back(std::move(*param));
    }

    while (is_one_of(token, TokenType::COMMA)) {
        advance();
        param = parse_SingleParameter();
        if (param) {
            list.push_back(std::move(*param));
        } else {
            report_expected_parameter();
        }
    }
    return list;
}

std::optional<FunctionDecl::Parameter> Parser::parse_SingleParameter()
{
    if (!is_one_of(token, TokenType::IDENTIFIER)) {
        return {};
    }
    std::wstring name = *get_string(token);
    const Position pos = token.position;
    advance();
    eat(L"Expected type declaration token `:`", TokenType::COLON);
    auto type = parse_Type();
    return FunctionDecl::Parameter{ std::move(name), type, pos };
}

std::unique_ptr<Block> Parser::parse_Block()
{
    eat(L"Expected `{` paren", TokenType::LS_PAREN);
    std::list<std::unique_ptr<Statement> > list;
    auto statement = parse_Statement();
    while (statement) {
        list.push_back(std::move(statement));
        statement = parse_Statement();
    }
    eat(L"Expected `}` paren", TokenType::RS_PAREN);
    return make<Block>(std::move(std::move(list)));
}

std::unique_ptr<Expression> Parser::parse_ConditionalExpression()
{
    auto node = parse_UnaryLogicalExpr();
    if (!node) {
        return nullptr;
    }
    while (is_boolean_binary_op(token)) {
        auto op = BinOp_from_token(token);
        auto position = token.position;
        advance();
        auto rhs = parse_UnaryLogicalExpr();
        if (!rhs) {
            return nullptr;
        }
        node = make<BinaryExpression>(position, op, std::move(node), std::move(rhs));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_UnaryLogicalExpr()
{
    if (is_one_of(token, TokenType::BOOLEAN_NEG)) {
        auto position = token.position;
        advance();
        auto lhs = parse_LogicalExpr();
        if (!lhs) {
            return nullptr;
        }
        return make<UnaryExpression>(position, UnaryOperator::BooleanNeg, std::move(lhs));
    } else {
        return parse_LogicalExpr();
    }
}

std::unique_ptr<Expression> Parser::parse_LogicalExpr()
{
    auto node = parse_ArithmeticalExpr();
    if (!node) {
        return nullptr;
    }
    while (is_compare_op(token)) {
        auto op = BinOp_from_token(token);
        auto position = token.position;
        advance();
        auto rhs = parse_ArithmeticalExpr();
        if (!rhs) {
            return nullptr;
        }
        node = make<BinaryExpression>(position, op, std::move(node), std::move(rhs));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_ArithmeticalExpr()
{
    auto node = parse_AdditiveExpr();
    if (!node) {
        return nullptr;
    }
    while (is_bitwise_op(token)) {
        auto op = BinOp_from_token(token);
        auto position = token.position;
        advance();
        auto rhs = parse_AdditiveExpr();
        if (!rhs) {
            return nullptr;
        }
        node = make<BinaryExpression>(position, op, std::move(node), std::move(rhs));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_AdditiveExpr()
{
    auto node = parse_MultiplicativeExpr();
    if (!node) {
        return nullptr;
    }
    while (is_additive_op(token)) {
        auto op = BinOp_from_token(token);
        auto position = token.position;
        advance();
        auto rhs = parse_MultiplicativeExpr();
        if (!rhs) {
            return nullptr;
        }
        node = make<BinaryExpression>(position, op, std::move(node), std::move(rhs));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_MultiplicativeExpr()
{
    auto node = parse_UnaryExpression();
    if (!node) {
        return nullptr;
    }
    while (is_multiplicative_op(token)) {
        auto op = BinOp_from_token(token);
        auto position = token.position;
        advance();
        auto rhs = parse_UnaryExpression();
        if (!rhs) {
            return nullptr;
        }
        node = make<BinaryExpression>(position, op, std::move(node), std::move(rhs));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_UnaryExpression()
{
    std::stack<std::pair<UnaryOperator, Position> > operators;
    while (is_unary_op(token)) {
        operators.push(std::make_pair(UnOp_from_token(token), token.position));
        advance();
    }
    auto lhs = parse_Factor();
    auto index_position = token.position;
    auto index = parse_IndexExpression();
    if (index) {
        lhs = make<IndexExpression>(index_position, std::move(lhs), std::move(index));
    }
    if (!lhs) {
        return nullptr;
    }
    while (!operators.empty()) {
        auto [op, position] = operators.top();
        operators.pop();
        lhs = make<UnaryExpression>(position, op, std::move(lhs));
    }
    return lhs;
}

std::unique_ptr<Expression> Parser::parse_Factor()
{
    auto int_const = parse_IntConst();
    if (int_const) {
        return int_const;
    }

    auto string_const = parse_StringConst();
    if (string_const) {
        return string_const;
    }

    auto expr = parse_FuncCallOrVariableRef();
    if (expr) {
        return expr;
    }

    auto nested = parse_NestedExpression();
    if (nested) {
        return nested;
    }

    return nullptr;
}

std::unique_ptr<IntConst> Parser::parse_IntConst()
{
    if (is_one_of(token, TokenType::INTCONST)) {
        auto value = *get_int(token);
        auto position = token.position;
        advance();
        return make<IntConst>(position, value);
    } else {
        return nullptr;
    }
}

std::unique_ptr<StringConst> Parser::parse_StringConst()
{
    if (is_one_of(token, TokenType::STRINGCONST)) {
        auto value = *get_string(token);
        auto position = token.position;
        advance();
        return make<StringConst>(position, value);
    } else {
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parse_FuncCallOrVariableRef()
{
    if (!is_one_of(token, TokenType::IDENTIFIER)) {
        return nullptr;
    }
    std::wstring name = *get_string(token);
    auto position = token.position;
    advance();
    auto func_call = parse_FunctionCall(position, name);
    if (func_call) {
        return func_call;
    } else {
        return make<VariableRef>(position, std::move(name));
    }
}

std::unique_ptr<Expression> Parser::parse_NestedExpression()
{
    if (!is_one_of(token, TokenType::L_PAREN)) {
        return nullptr;
    }
    advance();
    auto expr = parse_ConditionalExpression();
    eat(L"Expected closing paren `)` at the end of expression", TokenType::R_PAREN);
    return expr;
}

std::unique_ptr<FunctionCall> Parser::parse_FunctionCall(const Position &position, std::wstring name)
{
    if (!is_one_of(token, TokenType::L_PAREN)) {
        return nullptr;
    }
    advance();
    auto arguments = parse_CallArgumentList();
    eat(L"Expected closing paren `)` at the end of argument list", TokenType::R_PAREN);
    return make<FunctionCall>(position, std::move(name), std::move(arguments));
}

std::list<std::unique_ptr<Expression> > Parser::parse_CallArgumentList()
{
    std::list<std::unique_ptr<Expression> > list;
    auto node = parse_ArithmeticalExpr();
    if (node) {
        list.push_back(std::move(node));
        while (is_one_of(token, TokenType::COMMA)) {
            advance();
            node = parse_ArithmeticalExpr();
            if (!node) {
                report_expected_expression();
            }
            list.push_back(std::move(node));
        }
    }
    return list;
}

std::unique_ptr<Expression> Parser::parse_IndexExpression()
{
    if (!is_one_of(token, TokenType::LI_PAREN)) {
        return nullptr;
    }
    advance();
    auto index = parse_ArithmeticalExpr();
    eat(L"Expected closing `]` paren to end indexing", TokenType::RI_PAREN);
    return index;
}

std::unique_ptr<Statement> Parser::parse_Statement()
{
    auto for_stmt = parse_ForStatement();
    if (for_stmt) {
        return for_stmt;
    }

    auto while_stmt = parse_WhileStatement();
    if (while_stmt) {
        return while_stmt;
    }

    auto if_stmt = parse_IfStatement();
    if (if_stmt) {
        return if_stmt;
    }

    auto return_stmt = parse_ReturnSatetemnt();
    if (return_stmt) {
        return return_stmt;
    }

    auto var_decl = parse_VariableDecl();
    if (var_decl) {
        return var_decl;
    }

    auto assignment = parse_AssignStatement();
    if (assignment) {
        return assignment;
    }

    return nullptr;
}

std::unique_ptr<IfStatement> Parser::parse_IfStatement()
{
    if (!is_one_of(token, TokenType::KW_IF)) {
        return nullptr;
    }
    advance();

    std::list<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block> > > blocks;
    blocks.push_back(parse_ConditionalBlock());

    while (is_one_of(token, TokenType::KW_ELIF)) {
        advance();
        blocks.push_back(parse_ConditionalBlock());
    }

    std::optional<std::unique_ptr<Block> > else_statement;
    if (is_one_of(token, TokenType::KW_ELSE)) {
        advance();
        auto block = parse_Block();
        else_statement = std::move(block);
    }
    return make<IfStatement>(std::move(blocks), std::move(else_statement));
}

std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block> > Parser::parse_ConditionalBlock()
{
    auto condition = parse_ConditionalExpression();
    if (!condition) {
        report_expected_expression();
    }
    auto block = parse_Block();
    return std::make_pair(std::move(condition), std::move(block));
}

std::unique_ptr<ForStatement> Parser::parse_ForStatement()
{
    if (!is_one_of(token, TokenType::KW_FOR)) {
        return nullptr;
    }
    advance();
    expect(L"Expected loop's variable name", TokenType::IDENTIFIER);
    std::wstring name = *get_string(token);
    const auto pos = token.position;
    advance();
    eat(L"Expected `in` keyword", TokenType::KW_IN);
    auto [start, end, increase] = parse_Range();
    auto block = parse_Block();
    return make<ForStatement>(std::move(name), pos, std::move(start), std::move(end), std::move(increase),
                              std::move(block));
}

std::tuple<std::unique_ptr<Expression>, std::unique_ptr<Expression>, std::optional<std::unique_ptr<Expression> > >
Parser::parse_Range()
{
    auto start = parse_ArithmeticalExpr();
    if (!start) {
        report_expected_expression();
    }
    eat(L"Expected range separator `..`", TokenType::RANGE_SEP);
    auto end = parse_ArithmeticalExpr();
    if (!end) {
        report_expected_expression();
    }
    std::optional<std::unique_ptr<Expression> > increase;
    if (is_one_of(token, TokenType::RANGE_SEP)) {
        advance();
        auto inc = parse_ArithmeticalExpr();
        if (!inc) {
            report_expected_expression();
        }
        increase = std::move(inc);
    }
    return std::make_tuple(std::move(start), std::move(end), std::move(increase));
}

std::unique_ptr<WhileStatement> Parser::parse_WhileStatement()
{
    if (!is_one_of(token, TokenType::KW_WHILE)) {
        return nullptr;
    }
    advance();
    auto [expr, block] = parse_ConditionalBlock();
    return make<WhileStatement>(std::move(expr), std::move(block));
}

std::unique_ptr<ReturnStatement> Parser::parse_ReturnSatetemnt()
{
    if (!is_one_of(token, TokenType::KW_RETURN)) {
        return nullptr;
    }
    advance();
    auto expr = parse_ArithmeticalExpr();
    eat(L"Expected semicolon `;` et the end of return statement", TokenType::SEMICOLON);
    return make<ReturnStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::parse_AssignStatement()
{
    auto expr = parse_ConditionalExpression();
    if (!expr) {
        return nullptr;
    }
    if (is_one_of(token, TokenType::SEMICOLON)) {
        advance();
        return make<ExpressionStatement>(std::move(expr));
    }
    std::list<std::unique_ptr<Expression> > parts;
    parts.push_back(std::move(expr));
    while (is_one_of(token, TokenType::ASSIGN)) {
        advance();
        expr = parse_ConditionalExpression();
        if (!expr) {
            report_expected_expression();
        }
        parts.push_back(std::move(expr));
    }
    eat(L"Expected semicolon `;` at the end of assignment expression", TokenType::SEMICOLON);
    return make<AssignmentStatement>(std::move(parts));
}

void Parser::report_unexpected_token(const std::wstring &msg)
{
    const auto position = token.position;
    throw ParserException{ concat(position_in_file(position), L"\n In \n",
                                  lexer->get_lines(position.line_number, position.line_number + 1), L"\n",
                                  error_marker(position), L"\n", L"\nError unexpected token\n", msg,
                                  L"\n Got `\033[31;1;4m", repr(token.type), L"\033[0m`\n") };
}

void Parser::report_expected_expression()
{
    const auto position = token.position;
    throw ParserException{ concat(position_in_file(position), L"\n In \n",
                                  lexer->get_lines(position.line_number, position.line_number + 1), L"\n",
                                  error_marker(position), L"\n", L"\nExpected expression but got ", repr(token.type)) };
}

void Parser::report_invalid_type() const
{
    const auto position = token.position;
    throw ParserException{ concat(
        position_in_file(position), L"\n In \n", lexer->get_lines(position.line_number, position.line_number + 1),
        L"\n", error_marker(position), L"\n", L"Invalid type you can only use int, int* or string\n") };
}

void Parser::report_expected_parameter()
{
    const auto position = token.position;
    throw ParserException{ concat(position_in_file(position), L"\n In \n",
                                  lexer->get_lines(position.line_number, position.line_number + 1), L"\n",
                                  error_marker(position), L"\n",
                                  L"Expected parameter declaration starting with name but got", repr(token.type)) };
}
