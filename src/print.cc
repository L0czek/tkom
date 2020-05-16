#include "print.hpp"

#include "common.hpp"

PrintVisitor::PrintVisitor(std::size_t ident) : ident(ident), str(L"") {}

void PrintVisitor::visit(const UnaryExpression& target) {
	PrintVisitor visitor{ident + 1};
	target.rhs->accept(visitor);
	str = concat(make_identation(ident), repr(target.op), L" : {\n", visitor.result(), L"\n", make_identation(ident), L"}");
}

void PrintVisitor::visit(const BinaryExpression& target) {
	PrintVisitor lhs{ident + 1}, rhs{ident + 1};
	target.rhs->accept(rhs);
	target.lhs->accept(lhs);
	str = concat(make_identation(ident), repr(target.op), L" : {\n", lhs.result(), L"\n", rhs.result(), L"\n", make_identation(ident), L"}");
}

void PrintVisitor::visit(const IndexExpression& target) {
	PrintVisitor ptr{ident + 1};
	PrintVisitor index{ident + 1};
	target.ptr->accept(ptr);
	target.index->accept(index);
	str = concat(make_identation(ident), L"Index : {\n", ptr.result(), L"\n", index.result(), L"\n", make_identation(ident), L"}");
}

void PrintVisitor::visit(const VariableRef& target) {
	str = concat(make_identation(ident), L"[ get var `", target.var_name, L"` ]");
}

void PrintVisitor::visit(const FunctionCall& target) {
	str = concat(make_identation(ident), L"FunctionCall name = `", target.func_name, L"`; args = {\n");
	for (const auto &i : target.arguments) {
		PrintVisitor visitor{ident+1};
		i->accept(visitor);
		str += visitor.result() + L"\n";
	}
	str += make_identation(ident) + L"}";
}

void PrintVisitor::visit(const IntConst& target) {
	str = concat(make_identation(ident), L"[ int `", std::to_wstring(target.value), L"` ]");
}

void PrintVisitor::visit(const StringConst& target) {
	str = concat(make_identation(ident), L"[ string `", target.value, L"` ]");
}

void PrintVisitor::visit(const Block& target) {
	for (const auto &i : target.statements) {
		PrintVisitor visitor{ident + 1};
		i->accept(visitor);
		str += concat(visitor.result(), L",\n");
	}
}

void PrintVisitor::visit(const FunctionDecl& target) {
	str = concat(make_identation(ident), L"[ make function name = `", target.func_name, L"`; return type = `", repr(target.return_type), L"`; args = {\n");
	for (const auto &i : target.parameters) {
		str += concat(make_identation(ident + 2), L"name = `", i.name, L"`; type = `", repr(i.type), L"`,\n");
	}
	str += make_identation(ident+1) + L"}\n";
	str += make_identation(ident+1) + L"with body = {\n";
	PrintVisitor visitor{ident + 2};
	target.block->accept(visitor);
	str += visitor.result();
	str += concat(make_identation(ident+1), L"}\n", make_identation(ident), L"]");
}

void PrintVisitor::visit(const VariableDecl& target) {
	for (const auto &i : target.var_decls) {
		str += concat(make_identation(ident), L"[ make var `", i.name, L"` of type `", repr(i.type), L"`");
		if (i.initial_value) {
			PrintVisitor visitor{ident + 1};
			(*i.initial_value)->accept(visitor);
			str += concat(L" = \n", visitor.result());
		}
		str += L"\n" + make_identation(ident) + L"]";
	}
}

void PrintVisitor::visit(const AssignmentStatement& target) {
	str = concat(make_identation(ident), L"[ Assign parts = {\n");
	for (const auto &i : target.parts) {
		PrintVisitor part{ident + 2};
		i->accept(part);
		str += concat(part.result(), L"\n");
		str += concat(make_identation(ident+1), L"}, next = {\n");
	}
	str += concat(make_identation(ident+1), L"},\n", make_identation(ident), L"end Assign ]");
}

void PrintVisitor::visit(const ReturnStatement& target) {
	PrintVisitor visitor{ident + 1};
	target.expr->accept(visitor);
	str = concat(make_identation(ident), L"Return : {\n", visitor.result(), L"\n", make_identation(ident), L"}");
}

void PrintVisitor::visit(const ExpressionStatement& target) {
	PrintVisitor visitor{ident + 1};
	target.expr->accept(visitor);
	str = concat(visitor.result());
}

void PrintVisitor::visit(const IfStatement& target) {
	str = concat(make_identation(ident), L"[ if \n");
	for (const auto &i : target.blocks) {
		PrintVisitor cond{ident + 2};
		PrintVisitor block{ident + 2};
		i.first->accept(cond);
		i.second->accept(block);
		str += concat(make_identation(ident + 1), L"[ condition = {\n", cond.result(), L"\n", make_identation(ident+1), L"}\n");
		str += concat(make_identation(ident + 1), L"  block = {\n", block.result(), L"\n", make_identation(ident+1), L"} ],\n");
	}

	if (target.else_statement) {
		PrintVisitor else_stmt{ident + 2};
		(*target.else_statement)->accept(else_stmt);
		str += concat(make_identation(ident+1), L"[ else block = {\n", else_stmt.result(), L"\n", make_identation(ident+1), L"],\n");
	}

	str += concat(make_identation(ident), L" end if]");
}

void PrintVisitor::visit(const ForStatement& target) {
	str = concat(make_identation(ident), L"[ for loop variable name = `", target.loop_variable,L"`\n");
	PrintVisitor start{ident+2};
	PrintVisitor end{ident + 2};
	target.start->accept(start);
	target.end->accept(end);
	str += concat(make_identation(ident+1), L"start = {\n", start.result(), L"\n", make_identation(ident+1), L"},\n");
	str += concat(make_identation(ident+1), L"end = {\n", end.result(), L"\n", make_identation(ident+1), L"},\n");
	if (target.increase) {
		PrintVisitor inc{ident+2};
		(*target.increase)->accept(inc);
		str += concat(make_identation(ident+1), L"increase = {\n", inc.result(), L"\n", make_identation(ident+1), L"},\n");
	} else {
		str += concat(make_identation(ident+1), L"increase = default;\n");
	}
	PrintVisitor block{ident+2};
	target.block->accept(block);
	str += concat(make_identation(ident+1), L"with body = {\n", block.result(), L"\n", make_identation(ident+1), L"},\n");
	str += concat(make_identation(ident), L" end for ]");
}

void PrintVisitor::visit(const WhileStatement& target) {
	PrintVisitor condition{ident+2};
	PrintVisitor block{ident + 2};
	target.condition->accept(condition);
	target.block->accept(block);
	str = concat(make_identation(ident), L"[ while loop condition = {\n");
	str += concat(condition.result(), L"\n", make_identation(ident+1), L"},\n");
	str += concat(make_identation(ident+1), L"with body = {\n");
	str += concat(block.result(), L"\n", make_identation(ident+1), L"},\n");
	str += concat(make_identation(ident), L"end while ]");
}

void PrintVisitor::visit(const ExternFunctionDecl& target) {
	str = concat(make_identation(ident), L"[ get extern function name = `", target.func_name, L"`; return type = `", repr(target.return_type), L"`; args = {\n");
	for (const auto &i : target.parameters) {
		str += concat(make_identation(ident + 2), L"name = `", i.name, L"`; type = `", repr(i.type), L"`,\n");
	}
	str += make_identation(ident+1) + L"}\n";
    str += concat(make_identation(ident), L"]");
}

void PrintVisitor::visit(const Program& target) {
    for (const auto &i : target.externs) {
        PrintVisitor visitor{ident + 1};
        i->accept(visitor);
        str += visitor.result() + L"\n";
    }
    for (const auto &i : target.global_vars) {
		PrintVisitor visitor{ident + 1};
		i->accept(visitor);
		str += visitor.result() + L"\n";	
	}
	for (const auto &i : target.functions) {
		PrintVisitor visitor{ident + 1};
		i->accept(visitor);
		str += visitor.result() + L"\n";
	}
}


std::wstring PrintVisitor::result() {
	return std::move(str);
}

std::wstring repr(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Plus: return L"Plus";
        case BinaryOperator::Minus: return L"Minus";


        case BinaryOperator::Multiply: return L"Multiply";
        case BinaryOperator::Divide: return L"Divide";
        case BinaryOperator::Modulo: return L"Modulo";


        case BinaryOperator::And: return L"And";
        case BinaryOperator::Xor: return L"Xor";
        case BinaryOperator::Or: return L"Or";
        case BinaryOperator::ShiftLeft: return L"ShiftLeft";
        case BinaryOperator::ShiftRight: return L"ShiftRight";


        case BinaryOperator::Less: return L"Less";
        case BinaryOperator::Greater: return L"Greater";
        case BinaryOperator::LessEqual: return L"LessEqual";
        case BinaryOperator::GreaterEqual: return L"GreaterEqual";
        case BinaryOperator::Equal: return L"Equal";
        case BinaryOperator::NotEqual: return L"NotEqual";


        case BinaryOperator::BooleanAnd: return L"BooleanAnd";
        case BinaryOperator::BooleanOr: return L"BooleanOr";
    }
    throw std::runtime_error("no such operator");
}

std::wstring repr(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Minus: return L"Unary Minus";
        case UnaryOperator::Neg: return L"Neg";
        case UnaryOperator::Addrof: return L"Addrof";
        case UnaryOperator::BooleanNeg: return L"BooleanNeg";
        case UnaryOperator::Deref: return L"Deref";
    }
    throw std::runtime_error("no such operator");
}

std::wstring make_identation(std::size_t n) {
	std::wstring wstr;
	for (std::size_t i=0; i < n; ++i) {
		wstr += L"     ";
	}
	return wstr;
}

std::wstring repr(BuiltinType type) {
	switch (type) {
		case BuiltinType::Int: return L"Int";
		case BuiltinType::String: return L"String";
		case BuiltinType::IntPointer: return L"IntPointer";
	}
	throw std::runtime_error("no such type");
}
