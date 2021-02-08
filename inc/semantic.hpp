#ifndef __SEMANTIC_HPP__
#define __SEMANTIC_HPP__

#include "common.hpp"
#include "node.hpp"
#include "visitor.hpp"

#include <algorithm>
#include <deque>
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <unordered_set>

class SemanticAnalyser : public Visitor {
    std::unique_ptr<Source> source;

public:
    enum class ExprType { Int, String, IntPointer, IntPointerReference, IntReference, StringReference, Bool };

private:
    BuiltinType current_func_ret_type;

    struct Function {
        BuiltinType return_type;
        std::list<std::pair<std::wstring, BuiltinType> > parameters;
    };

    std::stack<std::pair<ExprType, Position> > stack;
    std::stack<bool> has_return;
    std::deque<std::unordered_map<std::wstring, BuiltinType> > scopes;
    std::unordered_map<std::wstring, Function> functions;

    void ignore_return(std::size_t depth);
    void yield_return();
    void yield_no_return();
    void yield_return_all(std::size_t depth);
    void yield_return_one(std::size_t depth);
    void assert_returns(const Position &pos);

    template <typename Node> void analyse(const std::unique_ptr<Node> &node);
    template <typename... Types> void require(Types &&... types);
    void ignore();
    template <typename... Types> bool is_one_of(ExprType first, Types &&... types);
    bool is_one_of(ExprType allowed);
    ExprType pop();
    void check_id(const std::wstring &name, const Position &position) const;

    void yield(ExprType type, const Position &pos);

    void enter();
    void leave();
    ExprType from_builtin_type(BuiltinType type);
    ExprType from_builtin_type_value(BuiltinType type);
    BuiltinType get_var(const VariableRef &var) const;
    void declare_var(const VariableDecl::SingleVarDecl &expr);
    bool check_var_name(const std::wstring &name);
    bool is_in_scope(const std::wstring &name, const std::unordered_map<std::wstring, BuiltinType> &variables) const;
    BuiltinType var_from_scope(const std::wstring &name,
                               const std::unordered_map<std::wstring, BuiltinType> &scope) const;
    const Function &function_from_name(const std::wstring &name, const Position &pos);
    void check_assignable_by(const std::unique_ptr<Expression> &expr, SemanticAnalyser::ExprType rhs);
    void check_assignable_by(BuiltinType type, const std::unique_ptr<Expression> &expr);
    void check_main_function(const FunctionDecl &decl);

    template <typename... Types>[[noreturn]] void report_bad_type(Types &&... allowed) const;
    [[noreturn]] void report_reserved_word(const std::wstring &word, const Position &pos) const;
    [[noreturn]] void report_undefined_variable(const std::wstring &name, const Position &pos) const;
    [[noreturn]] void report_undefined_function(const std::wstring &name, const Position &pos) const;
    [[noreturn]] void report_invalid_argument(ExprType expected, const Position &pos) const;
    [[noreturn]] void report_argument_number_mismatch(std::size_t expected, std::size_t got, const Position &pos) const;
    [[noreturn]] void report_variable_redeclaration(const std::wstring &name, const Position &pos) const;
    [[noreturn]] void report_function_redeclaration(const std::wstring &name, const Position &pos) const;
    [[noreturn]] void report_parameter_redeclaration(const std::wstring &name, const Position &pos) const;
    [[noreturn]] void report_no_return(const Position &position) const;
    [[noreturn]] void report_main_bad_params(const Position &pos) const;
    [[noreturn]] void report_main_bad_return_type(const Position &pos) const;

    template <typename... Types> static std::wstring repr(ExprType first, Types &&... types);
    static std::wstring repr(ExprType type);
    static const std::unordered_set<std::wstring> reserved_words;

public:
    SemanticAnalyser(std::unique_ptr<Source> source) : source(std::move(source))
    {
    }
    void visit(const UnaryExpression &) override;
    void visit(const BinaryExpression &) override;
    void visit(const IndexExpression &) override;
    void visit(const VariableRef &) override;
    void visit(const FunctionCall &) override;
    void visit(const IntConst &) override;
    void visit(const StringConst &) override;
    void visit(const Block &) override;
    void visit(const FunctionDecl &) override;
    void visit(const VariableDecl &) override;
    void visit(const AssignmentStatement &) override;
    void visit(const ReturnStatement &) override;
    void visit(const ExpressionStatement &) override;
    void visit(const IfStatement &) override;
    void visit(const ForStatement &) override;
    void visit(const WhileStatement &) override;
    void visit(const Program &) override;
    void visit(const ExternFunctionDecl &) override;
};

void analyse(const std::unique_ptr<Program> &program, std::unique_ptr<Source> source);

class SemanticException : public std::runtime_error {
    std::wstring msg;
    std::string ascii_msg;

public:
    SemanticException(const std::wstring &wstr)
        : std::runtime_error("ParserException"), msg(wstr), ascii_msg(to_ascii_string(msg))
    {
    }
    const std::wstring &message() const noexcept
    {
        return msg;
    }
    const char *what() const noexcept override
    {
        return ascii_msg.c_str();
    }
};

template <typename... Types> void SemanticAnalyser::require(Types &&... types)
{
    if (!is_one_of(types...)) {
        report_bad_type(types...);
    } else {
        stack.pop();
    }
}

template <typename... Types> bool SemanticAnalyser::is_one_of(ExprType first, Types &&... rest)
{
    return is_one_of(first) || is_one_of(rest...);
}

template <typename... Types> std::wstring SemanticAnalyser::repr(SemanticAnalyser::ExprType first, Types &&... types)
{
    std::wstring ret = concat(L"`", repr(first));
    if constexpr (sizeof...(Types) == 0) {
        return concat(ret, L"`");
    } else {
        return concat(ret, L"`,", repr(types...));
    }
}

template <typename... Allowed> void SemanticAnalyser::report_bad_type(Allowed &&... allowed) const
{
    const auto [got, position] = stack.top();
    throw SemanticException{ concat(position_in_file(position), L"\n In \n",
                                    source->get_lines(position.line_number, position.line_number + 1), L"\n",
                                    error_marker(position), L"\n\n", L"Error expected one of type `", repr(allowed...),
                                    L"` but instead got `", repr(got), L"`\n") };
}

template <typename Node> void SemanticAnalyser::analyse(const std::unique_ptr<Node> &node)
{
    node->accept(*this);
}

#endif
