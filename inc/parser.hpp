#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "lexer.hpp"
#include "node.hpp"

#include <stack>

class Parser {
    public: // TODO remove after completed debugging
    std::unique_ptr<Lexer> lexer;
    Token token;
    std::stack<UnaryOperator> stack;

    std::unique_ptr<Program> parse_Program();
    std::unique_ptr<FunctionDecl> parse_FunctionDecl();
    std::unique_ptr<VariableDecl> parse_VariableDecl();
    VariableDecl::SingleVarDecl parse_SingleVarDecl();
    BuiltinType parse_VarType();
    std::list<std::pair<std::wstring, BuiltinType>> parse_DeclArgumentList();
    std::unique_ptr<Block> parse_Block();

    std::unique_ptr<Expression> parse_ConditionalExpression(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_UnaryLogicalExpr(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_LogicalExpr(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_ArithmeticalExpr(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_AdditiveExpr(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_MultiplicativeExpr(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_UnaryExpression(std::unique_ptr<Expression> lhs=nullptr);
    std::unique_ptr<Expression> parse_Factor();
    std::unique_ptr<FunctionCall> parse_FunctionCall(std::wstring name);
    std::list<std::unique_ptr<Expression>> parse_CallArgumentList();
    std::unique_ptr<Expression> parse_IndexExpression(std::unique_ptr<Expression> ptr);
    std::unique_ptr<ExpressionStatement> parse_ExpressionStatement(std::unique_ptr<Expression> lhs=nullptr);

    std::unique_ptr<Expression> parse_MutableExpression();
    std::unique_ptr<Expression> parse_PointerExpr();

    std::unique_ptr<Statement> parse_Statement();
    std::unique_ptr<IfStatement> parse_IfStatement();
    std::unique_ptr<ForStatement> parse_ForStatement();
    std::unique_ptr<WhileStatement> parse_WhileStatement();
    std::unique_ptr<ReturnStatement> parse_ReturnSatetemnt();
    std::unique_ptr<Statement> parse_AssignStatement();

    void advance() noexcept;
    BuiltinType get_builtin_type(const std::wstring& wstr) const;

    void report_error(const Position& start, const Position& end, const std::wstring& error_msg);

    template<typename ... Types>
    void expect(const std::wstring& msg, Types&&... types);
public:
    Parser() = default;

    std::unique_ptr<Lexer> attach_lexer(std::unique_ptr<Lexer> lex) noexcept;
    
    std::unique_ptr<ASTNode> parse();
};

class ParserException :public std::runtime_error {
    std::wstring msg;
public:
    ParserException(const std::wstring& wstr) :msg(wstr), std::runtime_error("ParserException") {}
    const std::wstring& message() const noexcept { return msg; }
    const char* what() const noexcept override { return to_ascii_string(msg).c_str(); }
};

template<typename ... Types>
void Parser::expect(const std::wstring& msg, Types&&... types) {
    if(!is_one_of(token, std::forward<Types>(types)... )) {
        throw ParserException{concat(msg, L"\nUnexpected token", repr(token))};
    }
}

#endif
