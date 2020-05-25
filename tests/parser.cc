#include <gtest/gtest.h>
#include "print_visitor.hpp"
#include <algorithm>
#include  <cctype>
#define private public
#include "parser.hpp"

#define CAT(A, B)   A##B
#define W(A)  CAT(L, #A)
#define E(a,b) test_expr(a,b)

template<typename Result, typename Func> 
std::unique_ptr<Result> parse_stmt(const std::wstring& wstr, const Func& func);
std::unique_ptr<Expression> parse_expr(const std::wstring& wstr);
void test_expr(const std::wstring& input, std::wstring expected);
std::wstring repr(const std::unique_ptr<Expression>& expr);

TEST(Expression, Factor) {
    E(W(1), W((1)));
    E(W(a), W((a)));
}

TEST(Expression, Conditional) {
    E(W(a && b),        W( ((a) && (b)) ));
    E(W(a && b && c),   W( (((a) && (b)) && (c)) ));
    E(W(a || b),        W( ((a) || (b)) ));
    E(W(a || b || c),   W( (((a) || (b)) || (c)) ));
    E(W(a && b || c),   W( (((a) && (b)) || (c)) ));
}

TEST(Expression, UnaryLogical) {
    E(W(!a),        W(( !(a) )));
    E(W(! a && b),  W( ( (!(a)) && (b) ) ));
}

TEST(Expression, Logical) {
    E(W(a > b),         W(((a) > (b))));
    E(W(a > b > c),     W((((a) > (b)) > (c))));
    E(W(a < b),         W(((a) < (b))));
    E(W(a < b < c),     W((((a) < (b)) < (c))));
    E(W(a >= b),        W(((a) >= (b))));
    E(W(a >= b >= c),   W((((a) >= (b)) >= (c))));
    E(W(a <= b),        W(((a) <= (b))));
    E(W(a <= b <= c),   W((((a) <= (b)) <= (c))));
    E(W(a == b),        W(((a) == (b))));
    E(W(a == b == c),   W((((a) == (b)) == (c))));
    E(W(a != b),        W(((a) != (b))));
    E(W(a != b != c),   W((((a) != (b)) != (c))));
    E(W(a > b < c),     W((((a) > (b)) < (c))));
    E(W(a > b < c >= d <= e != f == g),     W(( ((((((a) > (b)) < (c)) >= (d)) <= (e)) != (f)) == (g))));
}

TEST(Expression, Arithmetical) {
    E(W(a & b),         W(((a) & (b))));
    E(W(a & b & c),     W((((a) & (b)) & (c))));
    E(W(a | b),         W(((a) | (b))));
    E(W(a | b | c),     W((((a) | (b)) | (c))));
    E(W(a ^ b),         W(((a) ^ (b))));
    E(W(a ^ b ^ c),     W((((a) ^ (b)) ^ (c))));
    E(W(a & b | c ^ d), W(((((a) & (b)) | (c)) ^ (d))));
}

TEST(Expression, Additive) {
    E(W(a + b),         W(((a) + (b))));
    E(W(a + b + c),     W((((a) + (b)) + (c))));
    E(W(a - b),         W(((a) - (b))));
    E(W(a - b - c),     W((((a) - (b)) - (c))));
    E(W(a + b - c + d), W(((((a) + (b)) - (c)) + (d))));
}

TEST(Expression, Multiplicative) {
    E(W(a * b),         W(((a) * (b))));
    E(W(a * b * c),     W((((a) * (b)) * (c))));
    E(W(a / b),         W(((a) / (b))));
    E(W(a / b / c),     W((((a) / (b)) / (c))));
    E(W(a % b),         W(((a) % (b))));
    E(W(a % b % c),     W((((a) % (b)) % (c))));
    E(W(a * b / c % d), W(((((a) * (b)) / (c)) % (d))));
}

TEST(Expression, UnaryExpression) {
    E(W(&a),    W((&(a))));
    E(W(*a),    W((*(a))));
    E(W(~a),    W((~(a))));
    E(W(~*&a),  W((~(*(&(a))))));
}

TEST(Expression, IndexExpression) {
    E(W(a[1]),      W( ((a)[(1)] )));
    E(W(a+b[1]),    W( ((a) + ((b)[(1)]))));
    E(W(*b[1]),     W( ( *((b)[(1)]) ) ));
}

TEST(Expression, FunctionCall) {
    E(W(f()),       W(( f() )));
    E(W(f(1)),      W(( f((1)) )));
    E(W(f(a+1,b)),  W(( f(((a) + (1)), (b)) )));
}

TEST(Expression, Combined) {
    E(W(!a && !b || c),     W(( ((!(a)) && ( !(b) ) ) || (c)) ));
    E(W(! a == b),          W(( !((a) == (b)) )));
    E(W(1&c == 3|a),        W(( ((1) & (c)) == ((3) | (a)) )));
    E(W(1 & b + c),         W(( (1) & ((b) + (c)) )));
    E(W(a + b * c),         W(( (a) + ((b) * (c)) )));
    E(W(*a / b),            W(( (*(a)) / (b) )));
    E(W(*f()),              W(( *(f()) )));
}

TEST(Statement, Assignment) {
    auto stmt = parse_stmt<Statement>(W(a=b=c;), &Parser::parse_AssignStatement);
    auto assign = dynamic_cast<AssignmentStatement*>(stmt.get());
    auto expected = { L"(a)", L"(b)", L"(c)" };
    auto it = assign->parts.begin();
    EXPECT_EQ(expected.size(), assign->parts.size());
    for (const auto & i : expected) {
        EXPECT_EQ(repr(*it++), i);
    }
}

TEST(Statement, If) {
    auto ifstmt = parse_stmt<IfStatement>(W(if a { b(); } elif c { d(); } else { f(); }), &Parser::parse_IfStatement);
    std::list<std::pair<std::wstring, std::wstring>> expected = {
        { L"(a)", L"(b())" },
        { L"(c)", L"(d())" }
    };
    auto expect_else = L"(f())";
    EXPECT_EQ(ifstmt->blocks.size(), expected.size());
    auto it = ifstmt->blocks.begin();
    for (const auto & i : expected) {
        auto & [ cond, block ] = *it++;
        EXPECT_EQ(repr(cond), i.first);
        EXPECT_EQ(block->statements.size(), 1);
        EXPECT_EQ(repr(dynamic_cast<ExpressionStatement*>(block->statements.front().get())->expr), i.second);
    }
    EXPECT_TRUE(ifstmt->else_statement);
    EXPECT_EQ(repr(dynamic_cast<ExpressionStatement*>((*ifstmt->else_statement)->statements.front().get())->expr), expect_else);
}

TEST(Statement, For) {
    auto forstmt = parse_stmt<ForStatement>(W(for i in a..b..c { d(); }), &Parser::parse_ForStatement);
    EXPECT_EQ(forstmt->loop_variable, L"i");
    EXPECT_EQ(repr(forstmt->start), L"(a)");
    EXPECT_EQ(repr(forstmt->end),   L"(b)");
    EXPECT_TRUE(forstmt->increase);
    EXPECT_EQ(repr(*forstmt->increase), L"(c)");
    EXPECT_EQ(forstmt->block->statements.size(), 1);
    EXPECT_EQ(repr(dynamic_cast<ExpressionStatement*>(forstmt->block->statements.front().get())->expr), L"(d())");
}

TEST(Statement, While) {
    auto whilestmt = parse_stmt<WhileStatement>(W(while a { b(); }), &Parser::parse_WhileStatement);
    EXPECT_EQ(repr(whilestmt->condition), L"(a)");
    EXPECT_EQ(whilestmt->block->statements.size(), 1);
    EXPECT_EQ(repr(dynamic_cast<ExpressionStatement*>(whilestmt->block->statements.front().get())->expr), L"(b())");
}

TEST(Statement, VariableDeclaration) {
    auto var_decl = parse_stmt<VariableDecl>(W(let a : int;), &Parser::parse_VariableDecl);
    EXPECT_EQ(var_decl->var_decls.size(), 1);
    EXPECT_EQ(var_decl->var_decls.front().name, L"a");
    EXPECT_EQ(var_decl->var_decls.front().type, BuiltinType::Int);
    EXPECT_FALSE(var_decl->var_decls.front().initial_value);

    var_decl = parse_stmt<VariableDecl>(L"let a=1, b=2 : int;)", &Parser::parse_VariableDecl);
    EXPECT_EQ(var_decl->var_decls.size(), 2);
    
    EXPECT_EQ(var_decl->var_decls.front().name, L"a");
    EXPECT_EQ(var_decl->var_decls.front().type, BuiltinType::Int);
    EXPECT_TRUE(var_decl->var_decls.front().initial_value);
    EXPECT_EQ(repr(*var_decl->var_decls.front().initial_value), L"(1)");

    EXPECT_EQ(var_decl->var_decls.back().name, L"b");
    EXPECT_EQ(var_decl->var_decls.back().type, BuiltinType::Int);
    EXPECT_TRUE(var_decl->var_decls.back().initial_value);
    EXPECT_EQ(repr(*var_decl->var_decls.back().initial_value), L"(2)");
}

TEST(Statement, FunctionDeclaration) {
    auto func = parse_stmt<FunctionDecl>(L"fn a(b : int, c : int) -> int { d(); })", &Parser::parse_FunctionDecl);
    EXPECT_EQ(func->func_name, L"a");

    EXPECT_EQ(func->parameters.size(), 2);
    EXPECT_EQ(func->parameters.front().name, L"b");
    EXPECT_EQ(func->parameters.front().type, BuiltinType::Int);
    EXPECT_EQ(func->parameters.back().name, L"c");
    EXPECT_EQ(func->parameters.back().type, BuiltinType::Int);

    EXPECT_EQ(func->return_type, BuiltinType::Int);
    EXPECT_EQ(repr(dynamic_cast<ExpressionStatement*>(func->block->statements.front().get())->expr), L"(d())");
}

TEST(Statement, ExternFunctionDeclaration) {
    auto stmt = parse_stmt<ExternFunctionDecl>(L"extern fn malloc(size : int) -> int*;", &Parser::parse_ExternFunctionDecl);
    EXPECT_EQ(stmt->func_name, L"malloc");
    EXPECT_EQ(stmt->return_type, BuiltinType::IntPointer);
    EXPECT_EQ(stmt->parameters.size(), 1);
    EXPECT_EQ(stmt->parameters.front().name, L"size");
    EXPECT_EQ(stmt->parameters.front().type, BuiltinType::Int);
}

TEST(Invalid, VariableDeclarations) {
    EXPECT_THROW(parse_stmt<Statement>(L"let a : ;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let a, : int;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let  : int;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let a=,b : int;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let a int;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let a : invalid_type;", &Parser::parse_VariableDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"let a,b, : int;", &Parser::parse_VariableDecl), std::runtime_error);
}

TEST(Invalid, FunctionDeclaration) {
    EXPECT_THROW(parse_stmt<Statement>(L"fn f() ->  { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(,) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(b) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(b : ) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(b int) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(b : int,) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f() -> invalid_type { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(: int) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f() int { }", &Parser::parse_FunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"fn f(a : invalid_type) -> int { }", &Parser::parse_FunctionDecl), std::runtime_error);
}

TEST(Invalid, ExternFunctionDeclaration) {
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f() int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f() -> invalid_type;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f(a) -> int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f(a : int,) -> int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f(a : int, b) -> int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f(a,b) -> int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f(,) -> int;", &Parser::parse_ExternFunctionDecl), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"extern fn f() -> int", &Parser::parse_ExternFunctionDecl), std::runtime_error);
}

TEST(Invalid, ForStatement) {
    EXPECT_THROW(parse_stmt<Statement>(L"for i in 0..1 ", &Parser::parse_ForStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"for i 0..1 {  }", &Parser::parse_ForStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"for in 0..1 {  }", &Parser::parse_ForStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"for i in 0..1.. {  }", &Parser::parse_ForStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"for i in 0..1..2 ", &Parser::parse_ForStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"for 0..1 {  }", &Parser::parse_ForStatement), std::runtime_error);
}

TEST(Invalid, WhileStatement) {
    EXPECT_THROW(parse_stmt<Statement>(L"while 1 {  ", &Parser::parse_WhileStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"while  {  }", &Parser::parse_WhileStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"while 1", &Parser::parse_WhileStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"while ", &Parser::parse_WhileStatement), std::runtime_error);
}

TEST(Invalid, IfStatement) {
    EXPECT_THROW(parse_stmt<Statement>(L"if a { } elif b {  else { }", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if a elif b { } else { }", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if { } elif b { } else { }", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if a { } elif { } else { }", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if a { } elif b else { }", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if a { } elif b { } else", &Parser::parse_IfStatement), std::runtime_error);
    EXPECT_THROW(parse_stmt<Statement>(L"if a { } elif  else { }", &Parser::parse_IfStatement), std::runtime_error);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

std::unique_ptr<Expression> parse_expr(const std::wstring& wstr) {
    auto source = Source::from_wstring(wstr);
    auto lexer = Lexer::from_source(std::move(source));
    Parser parser;
    parser.attach_lexer(std::move(lexer));
    return parser.parse_ConditionalExpression();
}

std::wstring repr(const std::unique_ptr<Expression>& expr) {
    print_visitor_expr visitor;
    expr->accept(visitor);
    return visitor.result();
}

void test_expr(const std::wstring& input, std::wstring expected) {
    expected.erase(std::remove(expected.begin(), expected.end(), L' '), expected.end());
    EXPECT_EQ(repr(parse_expr(input)), expected);
}

template<typename Result, typename Func> 
std::unique_ptr<Result> parse_stmt(const std::wstring& wstr, const Func& func){
    auto source = Source::from_wstring(wstr);
    auto lexer = Lexer::from_source(std::move(source));
    Parser parser;
    parser.attach_lexer(std::move(lexer));
    return std::invoke(func, &parser);
}
