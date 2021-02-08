#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "lexer.hpp"
#include "node.hpp"

#include <stack>

class Parser {
    std::unique_ptr<Lexer> lexer;
    Token token;
private:
    std::unique_ptr<Program> parse_Program();
    std::unique_ptr<ExternFunctionDecl> parse_ExternFunctionDecl();
    std::unique_ptr<FunctionDecl> parse_FunctionDecl();
    std::unique_ptr<VariableDecl> parse_VariableDecl();
    VariableDecl::SingleVarDecl parse_SingleVarDecl();
    BuiltinType parse_Type();
    std::list<FunctionDecl::Parameter> parse_ParameterList();
    std::optional<FunctionDecl::Parameter> parse_SingleParameter();
    std::unique_ptr<Block> parse_Block();

    std::unique_ptr<Expression> parse_ConditionalExpression();
    std::unique_ptr<Expression> parse_UnaryLogicalExpr();
    std::unique_ptr<Expression> parse_LogicalExpr();
    std::unique_ptr<Expression> parse_ArithmeticalExpr();
    std::unique_ptr<Expression> parse_AdditiveExpr();
    std::unique_ptr<Expression> parse_MultiplicativeExpr();
    std::unique_ptr<Expression> parse_UnaryExpression();
    std::unique_ptr<Expression> parse_Factor();
    std::unique_ptr<FunctionCall> parse_FunctionCall(const Position& position, std::wstring name);
    std::list<std::unique_ptr<Expression>> parse_CallArgumentList();
    std::unique_ptr<Expression> parse_IndexExpression();
    std::unique_ptr<ExpressionStatement> parse_ExpressionStatement();
    std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block>> parse_ConditionalBlock();

    std::unique_ptr<IntConst> parse_IntConst();
    std::unique_ptr<StringConst> parse_StringConst();
    std::unique_ptr<Expression> parse_FuncCallOrVariableRef();
    std::unique_ptr<Expression> parse_NestedExpression();

    std::unique_ptr<Statement> parse_Statement();
    std::unique_ptr<IfStatement> parse_IfStatement();
    std::unique_ptr<ForStatement> parse_ForStatement();
    std::tuple<std::unique_ptr<Expression>, std::unique_ptr<Expression>, std::optional<std::unique_ptr<Expression>>> parse_Range();
    std::unique_ptr<WhileStatement> parse_WhileStatement();
    std::unique_ptr<ReturnStatement> parse_ReturnSatetemnt();
    std::unique_ptr<Statement> parse_AssignStatement();

    void advance() noexcept;
    BuiltinType get_builtin_type(const std::wstring& wstr) const;
    
    [[ noreturn ]] void report_unexpected_token(const std::wstring& msg);
    [[ noreturn ]] void report_expected_expression();
    [[ noreturn ]] void report_invalid_type() const;
    [[ noreturn ]] void report_expected_parameter();

    [[ noreturn ]] void report_error(const Position& start, const Position& end, const std::wstring& error_msg);

    template<typename ... Types>
    void expect(const std::wstring& msg, Types&&... types);

    template<typename ... Types>
    void eat(const std::wstring& msg, Types&&... types);
public:
    Parser() = default;

    std::unique_ptr<Lexer> attach_lexer(std::unique_ptr<Lexer> lex) noexcept;
    std::unique_ptr<Lexer> detach_lexer() noexcept; 
    std::unique_ptr<Program> parse();
};

class ParserException :public std::runtime_error {
    std::wstring msg;
    std::string ascii_msg;
public:
    ParserException(const std::wstring& wstr) :std::runtime_error("ParserException"), msg(wstr), ascii_msg(to_ascii_string(msg)) {}
    const std::wstring& message() const noexcept { return msg; }
    const char* what() const noexcept override { return ascii_msg.c_str(); }
};

template<typename ... Types>
void Parser::expect(const std::wstring& msg, Types&&... types) {
    if(!is_one_of(token, std::forward<Types>(types)... )) {
        report_unexpected_token(msg);
    }
}

template<typename ... Types>
void Parser::eat(const std::wstring& msg, Types&&... types) {
    expect(msg, std::forward<Types>(types)...);
    advance();
}

#endif
