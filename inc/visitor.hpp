#ifndef __VISIOR_HPP__
#define __VISIOR_HPP__

class UnaryExpression;
class BinaryExpression;
class IndexExpression;
class VariableRef;
class FunctionCall;
class IntConst;
class StringConst;
class Block;
class FunctionDecl;
class VariableDecl;
class AssignmentStatement;
class ReturnStatement;
class ExpressionStatement;
class IfStatement;
class ForStatement;
class WhileStatement;
class Program;
class ExternFunctionDecl;

class Visitor {
public:
    virtual void visit(const UnaryExpression &) = 0;
    virtual void visit(const BinaryExpression &) = 0;
    virtual void visit(const IndexExpression &) = 0;
    virtual void visit(const VariableRef &) = 0;
    virtual void visit(const FunctionCall &) = 0;
    virtual void visit(const IntConst &) = 0;
    virtual void visit(const StringConst &) = 0;
    virtual void visit(const Block &) = 0;
    virtual void visit(const FunctionDecl &) = 0;
    virtual void visit(const VariableDecl &) = 0;
    virtual void visit(const AssignmentStatement &) = 0;
    virtual void visit(const ReturnStatement &) = 0;
    virtual void visit(const ExpressionStatement &) = 0;
    virtual void visit(const IfStatement &) = 0;
    virtual void visit(const ForStatement &) = 0;
    virtual void visit(const WhileStatement &) = 0;
    virtual void visit(const Program &) = 0;
    virtual void visit(const ExternFunctionDecl &) = 0;
};

#endif
