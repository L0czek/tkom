#ifndef __PRINT_HPP__
#define __PRINT_HPP__

#include "visitor.hpp"
#include "node.hpp"
#include <string>

class PrintVisitor :public Visitor {
	std::size_t ident;
	std::wstring str;
public:
	PrintVisitor(std::size_t ident = 0);

	void visit(const UnaryExpression& ) override;
	void visit(const BinaryExpression& ) override;
	void visit(const IndexExpression& ) override;
	void visit(const VariableRef& ) override;
	void visit(const FunctionCall& ) override;
	void visit(const IntConst& ) override;
	void visit(const StringConst& ) override;
	void visit(const Block& ) override;
	void visit(const FunctionDecl& ) override;
	void visit(const VariableDecl& ) override;
	void visit(const AssignmentStatement& ) override;
	void visit(const ReturnStatement& ) override;
	void visit(const ExpressionStatement& ) override;
	void visit(const IfStatement& ) override;
	void visit(const ForStatement& ) override;
	void visit(const WhileStatement& ) override;
	void visit(const Program& ) override;
    void visit(const ExternFunctionDecl& ) override;

	std::wstring result();
};

std::wstring repr(BinaryOperator op);
std::wstring repr(UnaryOperator op);
std::wstring repr(BuiltinType type);

std::wstring make_identation(std::size_t n);

#endif
