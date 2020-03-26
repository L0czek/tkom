#include <gtest/gtest.h>

#include "lexer.hpp"
#include <initializer_list>

#define T(type) make_token(TokenType::type, Position{})
#define V(type, value) make_token(TokenType::type, Position{}, value)
#define CAT(A, B)   A##B
#define W(A)  CAT(L, #A)

bool operator!=(const Token& a, const Token& b) {
    return a.type != b.type || a.value != b.value;
}

bool operator==(const Token& a, const Token& b) {
    return a.type == b.type && a.value == b.value;
}


bool check_tokens(const std::wstring& test, std::initializer_list<Token> tokens) {
    Lexer lexer{ Source::from_wstring(test) };
    for (const auto &i : tokens) {
        const Token result = lexer.next();
        if (result != i) {
            return false;
        }
    }
    Token last = lexer.next();
    return last == T(END_OF_FILE);
}

#define CASE(str, tokens) EXPECT_TRUE(check_tokens(str, tokens))
#define LIST(...) { __VA_ARGS__ } 

TEST(Expression, Operators) {
    CASE(W(a+b),        LIST( V(IDENTIFIER, W(a)), T(PLUS), V(IDENTIFIER, W(b)) ));
    CASE(W(a+b-c),      LIST( V(IDENTIFIER, W(a)), T(PLUS), V(IDENTIFIER, W(b)), T(MINUS), V(IDENTIFIER, W(c)) ));
    CASE(W(a*b-c),      LIST( V(IDENTIFIER, W(a)), T(STAR), V(IDENTIFIER, W(b)), T(MINUS), V(IDENTIFIER, W(c)) ));
    CASE(W(a*b/c),      LIST( V(IDENTIFIER, W(a)), T(STAR), V(IDENTIFIER, W(b)), T(DIVIDE), V(IDENTIFIER, W(c)) ));
    CASE(W(a&b%c),      LIST( V(IDENTIFIER, W(a)), T(AMPERSAND), V(IDENTIFIER, W(b)), T(MODULO), V(IDENTIFIER, W(c)) ));
    CASE(W(a^b|c),      LIST( V(IDENTIFIER, W(a)), T(XOR), V(IDENTIFIER, W(b)), T(BIT_OR), V(IDENTIFIER, W(c)) ));
    CASE(W(a||b&&c),    LIST( V(IDENTIFIER, W(a)), T(BOOLEAN_OR), V(IDENTIFIER, W(b)), T(BOOLEAN_AND), V(IDENTIFIER, W(c)) ));
    CASE(W(a==b!=c),    LIST( V(IDENTIFIER, W(a)), T(EQUAL), V(IDENTIFIER, W(b)), T(NOT_EQUAL), V(IDENTIFIER, W(c)) ));
    CASE(W(a>b<c),      LIST( V(IDENTIFIER, W(a)), T(GREATER), V(IDENTIFIER, W(b)), T(LESS), V(IDENTIFIER, W(c)) ));
    CASE(W(a<<b>>c),    LIST( V(IDENTIFIER, W(a)), T(SHIFT_LEFT), V(IDENTIFIER, W(b)), T(SHIFT_RIGHT), V(IDENTIFIER, W(c)) ));
    CASE(W(a<=b>=c),    LIST( V(IDENTIFIER, W(a)), T(LESS_EQUAL), V(IDENTIFIER, W(b)), T(GREATER_EQUAL), V(IDENTIFIER, W(c)) ));
}

TEST(Statement, VarDeclaration) {
    CASE(W(let a=1 : int;), LIST( T(KW_LET), V(IDENTIFIER, W(a)), T(ASSIGN), V(INTCONST, 1), T(COLON), V(IDENTIFIER, W(int)), T(SEMICOLON) ));
    CASE(W(let a="str" : string;), LIST( T(KW_LET), V(IDENTIFIER, W(a)), T(ASSIGN), V(STRINGCONST, W(str)), T(COLON), V(IDENTIFIER, W(string)), T(SEMICOLON) ));
}

TEST(Statement, While) {
    CASE(W(while i < 10 { ; }), LIST( T(KW_WHILE), V(IDENTIFIER, W(i)), T(LESS), V(INTCONST, 10), T(LS_PAREN), T(SEMICOLON), T(RS_PAREN) ));
}

TEST(Statement, If) {
    CASE(W(if i > 0 { ; } elif i < -1 { ; } else { ; }),
            LIST(
                    T(KW_IF), V(IDENTIFIER, W(i)), T(GREATER), V(INTCONST, 0), T(LS_PAREN), T(SEMICOLON), T(RS_PAREN),
                    T(KW_ELIF), V(IDENTIFIER, W(i)), T(LESS), T(MINUS), V(INTCONST, 1),  T(LS_PAREN), T(SEMICOLON), T(RS_PAREN),
                    T(KW_ELSE),  T(LS_PAREN), T(SEMICOLON), T(RS_PAREN),
                )
            );
}

TEST(Statement, For) {
    CASE(W(for i in 0..10..2 { ; }), LIST(
                    T(KW_FOR), V(IDENTIFIER, W(i)), T(KW_IN), V(INTCONST, 0), T(RANGE_SEP), V(INTCONST, 10), T(RANGE_SEP), V(INTCONST, 2), T(LS_PAREN), T(SEMICOLON), T(RS_PAREN)
                ));
}

TEST(Function, Declaration) {
    CASE(W(fn a(b : int) -> int { ; }), LIST(
                    T(KW_FN), V(IDENTIFIER, W(a)), T(L_PAREN), V(IDENTIFIER,W(b)), T(COLON), V(IDENTIFIER,W(int)), T(R_PAREN),
                    T(TYPE_DECL), V(IDENTIFIER, W(int)), T(LS_PAREN), T(SEMICOLON), T(RS_PAREN)
                ));
}

TEST(Function, Call) {
    CASE(W(func(1,2,"11")), LIST(V(IDENTIFIER, W(func)), T(L_PAREN), V(INTCONST, 1), T(COMMA), V(INTCONST, 2), T(COMMA),
                V(STRINGCONST, W(11)), T(R_PAREN)));
}

TEST(Other, SpaceAndComments) {
    CASE(W(  q #nd32ndiu32nd\n #emimfif\na\n   #imdi enie\nc#minddinn), LIST(V(IDENTIFIER,W(q)), V(IDENTIFIER, W(a)), 
                V(IDENTIFIER,W(c))));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
