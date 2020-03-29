#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <list>
#include <memory>
#include <utility>
#include <optional>
#include <string>
#include "token.hpp"
#include "visitor.hpp"

struct ASTNode {
    virtual void accept(Visitor& ) const =0;
};

enum class BinaryOperator {
    Plus,
    Minus,

    Multiply,
    Divide,
    Modulo,

    And,
    Xor,
    Or,
    ShiftLeft,
    ShiftRight,

    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Equal,
    NotEqual,

    BooleanAnd,
    BooleanOr
};

enum class UnaryOperator {
    Minus,
    Neg,
    Addrof,
    Deref,
    
    BooleanNeg,
};


BinaryOperator BinOp_from_token(const Token& token);
UnaryOperator UnOp_from_token(const Token& token);

enum class BuiltinType {
    Int,
    String,

    IntPointer
};

struct Expression :public ASTNode {};

struct UnaryExpression :public Expression {
    UnaryOperator op;
    std::unique_ptr<Expression> rhs;
public:
    UnaryExpression(UnaryOperator op, std::unique_ptr<Expression> rhs) 
    :op(op), rhs(std::move(rhs)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct BinaryExpression :public Expression {
    BinaryOperator op;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
public:
    BinaryExpression(BinaryOperator op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs) 
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct IndexExpression :public Expression {
    std::unique_ptr<Expression> ptr;
    std::unique_ptr<Expression> index;
public:
    IndexExpression(std::unique_ptr<Expression> ptr, std::unique_ptr<Expression> index)
    : ptr(std::move(ptr)), index(std::move(index)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};  

struct VariableRef :public Expression {
    std::wstring var_name;
public:
    VariableRef(std::wstring name) : var_name(name) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct FunctionCall :public Expression {
    std::wstring func_name;
    std::list<std::unique_ptr<Expression>> arguments;
public:
    FunctionCall(std::wstring func_name, std::list<std::unique_ptr<Expression>> arguments) 
    : func_name(std::move(func_name)), arguments(std::move(arguments)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct IntConst :public Expression {
    int value;
public:
    IntConst(int value) : value(value) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct StringConst :public Expression {
    std::wstring value;
public:
    StringConst(std::wstring value) : value(std::move(value)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct Statement :public ASTNode {};

struct Block :public ASTNode {
    std::list<std::unique_ptr<Statement>> statements;
public:
    Block(std::list<std::unique_ptr<Statement>> statements) 
    : statements(std::move(statements)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct FunctionDecl :public Statement {
    std::wstring func_name;
    BuiltinType return_type;
    std::list<std::pair<std::wstring, BuiltinType>> arguments;
    std::unique_ptr<Block> block;
public:
    FunctionDecl(std::wstring func_name, BuiltinType return_type, std::list<std::pair<std::wstring, BuiltinType>> arguments, std::unique_ptr<Block> block)
    : func_name(std::move(func_name)), return_type(return_type), arguments(std::move(arguments)), block(std::move(block)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct VariableDecl :public Statement {
    struct SingleVarDecl {
        std::wstring name;
        BuiltinType type;
        std::optional<std::unique_ptr<Expression>> initial_value;
    };

    std::list<SingleVarDecl> var_decls;

    typedef std::list<SingleVarDecl> VarDeclList;
    
public:
    VariableDecl(std::list<SingleVarDecl> var_decls)
    : var_decls(std::move(var_decls)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct AssignmentStatement :public Statement {
    std::list<std::unique_ptr<Expression>> parts;
public:
    AssignmentStatement(std::list<std::unique_ptr<Expression>> parts) 
    : parts(std::move(parts)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct ReturnStatement :public Statement {
    std::unique_ptr<Expression> expr;
public:
    ReturnStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct ExpressionStatement :public Statement {
    std::unique_ptr<Expression> expr;
public:
    ExpressionStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct IfStatement :public Statement {
    std::list<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block>>> blocks;
    std::optional<std::unique_ptr<Block>> else_statement;
public:
    IfStatement(std::list<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Block>>> blocks, std::optional<std::unique_ptr<Block>> else_statement)
    : blocks(std::move(blocks)), else_statement(std::move(else_statement)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct ForStatement :public Statement {
    std::wstring loop_variable;
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> end;
    std::optional<std::unique_ptr<Expression>> increase;
    std::unique_ptr<Block> block;
public:
    ForStatement(std::wstring loop_variable, std::unique_ptr<Expression> start, std::unique_ptr<Expression> end, std::optional<std::unique_ptr<Expression>> increase,std::unique_ptr<Block> block)
    : loop_variable(std::move(loop_variable)), start(std::move(start)), end(std::move(end)), increase(std::move(increase)), block(std::move(block)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct WhileStatement :public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> block;
public:
    WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Block> block)
    : condition(std::move(condition)), block(std::move(block)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};

struct Program :public ASTNode {
    std::list<std::unique_ptr<VariableDecl>> global_vars;
    std::list<std::unique_ptr<FunctionDecl>> functions;
public:
    Program(std::list<std::unique_ptr<VariableDecl>> global_vars, std::list<std::unique_ptr<FunctionDecl>> functions)
    : global_vars(std::move(global_vars)), functions(std::move(functions)) {}
    void accept(Visitor& visitor) const override { visitor.visit(*this); }
};


#endif
