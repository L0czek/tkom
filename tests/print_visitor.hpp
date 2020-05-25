#ifndef __PRINT_VISITOR_HPP__
#define __PRINT_VISITOR_HPP__

#include "visitor.hpp"
#include "node.hpp"
#include "common.hpp"
#include <string>

#define OP(op) return #op;

class print_visitor_expr :public Visitor {
    std::wstring str;
public:
    void visit(const UnaryExpression& expr) override {
        print_visitor_expr visitor;
        expr.rhs->accept(visitor);
        str = concat(L"(", repr(expr.op), visitor.result(), L")"); 
    }
	
    void visit(const BinaryExpression& expr) override { 
        print_visitor_expr lhs, rhs;
        expr.lhs->accept(lhs);
        expr.rhs->accept(rhs);
        str = concat(L"(", lhs.result(), repr(expr.op), rhs.result(), L")");
    }

    void visit(const IndexExpression& expr) override { 
        print_visitor_expr ptr, index;
        expr.ptr->accept(ptr);
        expr.index->accept(index);
        str = concat(L"(", ptr.result(), L"[", index.result(), L"]",L")");
    }

    void visit(const VariableRef& expr) override { 
        str = concat(L"(", expr.var_name, L")");
    }

    void visit(const FunctionCall& expr) override { 
        str = concat(L"(", expr.func_name, L"(");
        for (const auto & arg : expr.arguments) {
            print_visitor_expr visitor;
            arg->accept(visitor);
            str += concat(visitor.result(), L",");
        }
        if (str.back() == L',') {
            str.erase(--str.end());
        }
        str += L"))";
    }

    void visit(const IntConst& expr) override { 
        str = concat(L"(", std::to_wstring(expr.value), L")");
    }

	void visit(const StringConst& expr) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const Block& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const FunctionDecl& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const VariableDecl& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const AssignmentStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const ReturnStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const ExpressionStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const IfStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const ForStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const WhileStatement& ) override { throw std::runtime_error("DO NOT USE"); }
	void visit(const Program& ) override { throw std::runtime_error("DO NOT USE"); }
    void visit(const ExternFunctionDecl&) override { throw std::runtime_error("DO NOT USE"); }

    std::wstring result() { return str; }

    wchar_t repr(UnaryOperator op) {
        switch (op) {
            case UnaryOperator::Minus: return L'-';
            case UnaryOperator::Neg: return L'~';
            case UnaryOperator::Addrof: return L'&';
            case UnaryOperator::Deref: return L'*';
            case UnaryOperator::BooleanNeg: return L'!';
        }
    }
    std::wstring repr(BinaryOperator op) {
        switch (op) {
            case BinaryOperator::Plus: return L"+"; 
            case BinaryOperator::Minus: return L"-";

            case BinaryOperator::Multiply: return L"*";
            case BinaryOperator::Divide: return L"/";
            case BinaryOperator::Modulo: return L"%";

            case BinaryOperator::And: return L"&";
            case BinaryOperator::Xor: return L"^";
            case BinaryOperator::Or: return L"|";
            case BinaryOperator::ShiftLeft: return L"<<";
            case BinaryOperator::ShiftRight: return L">>";

            case BinaryOperator::Less: return L"<";
            case BinaryOperator::Greater: return L">";
            case BinaryOperator::LessEqual: return L"<=";
            case BinaryOperator::GreaterEqual: return L">=";
            case BinaryOperator::Equal: return L"==";
            case BinaryOperator::NotEqual: return L"!=";

            case BinaryOperator::BooleanAnd: return L"&&";
            case BinaryOperator::BooleanOr: return L"||";
        }
    }
};



#endif
