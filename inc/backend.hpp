#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"

#include "visitor.hpp"
#include "node.hpp"
#include "common.hpp"

#include <memory>
#include <vector>
#include <stack>
#include <deque>
#include <unordered_map>

extern std::string default_data_layout;
extern std::string default_target_triple;

class LLVMCompiler :public Visitor {
    static llvm::LLVMContext ctx;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    std::string data_layout_str;
    std::string target_triple;
    const llvm::DataLayout data_layout;

    struct Function {
        std::vector<llvm::Type*> parameters;
        llvm::FunctionType* type;
        llvm::Function* llvm_ptr;
    };
    llvm::Function* current_function = nullptr;

    struct Variable {
        llvm::Type* type;
        llvm::Value* ptr;
    };

    std::stack<std::pair<lazyValue<llvm::Value*>, lazyValue<llvm::Value*>>> expressions;
    std::deque<std::unordered_map<std::wstring, Variable>> scopes;
    std::unordered_map<std::wstring, Function> functions;
    std::unordered_map<std::wstring, llvm::GlobalVariable*> global_vars;
    
    void enter();
    void leave();
    void create_variable(const std::wstring& name, llvm::Value* ptr, llvm::Type* type);
    void declare_variable(const std::wstring& name, llvm::Value* ptr, llvm::Type* type);
    llvm::Value* get_variable_ptr(const std::wstring& name);
    llvm::Type* get_variable_type(const std::wstring& name);
    const Variable& find_variable(const std::wstring& name);

    void yield(lazyValue<llvm::Value*> value, lazyValue<llvm::Value*> address=nullptr);
    llvm::Type* from_builtin_type(BuiltinType type);

    std::size_t current_register;
    std::wstring next_reg() noexcept;
    void reset_reg() noexcept;

    template<typename Node>
    void compile(const std::unique_ptr<Node>& node);

    template<typename Node>
    llvm::Value* compile_expr_val(const std::unique_ptr<Node>& node);

    template<typename Node>
    llvm::Value* compile_expr_ptr(const std::unique_ptr<Node>& node);
   
    template<typename Node>
    std::pair<lazyValue<llvm::Value*>, lazyValue<llvm::Value*>> compile_expr(const std::unique_ptr<Node>& node);

    void declare_global_var(const std::unique_ptr<VariableDecl>& stmt);
    void declare_global_var(const std::wstring& name, BuiltinType type);

    void process_parameters(const std::list<FunctionDecl::Parameter>& parameters, llvm::Function* function);
    void optimize();
public:
    LLVMCompiler(const std::string& target, const std::string& data_layout);
    ~LLVMCompiler() {
        
    }

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
};

std::wstring compile(
        const std::unique_ptr<Program>& program, 
        const std::string& target = default_target_triple, 
        const std::string& data_layout = default_data_layout
    );

template<typename Node>
void LLVMCompiler::compile(const std::unique_ptr<Node>& node) {
    node->accept(*this);
}

template<typename Node>
llvm::Value* LLVMCompiler::compile_expr_val(const std::unique_ptr<Node>& node) {
    return compile_expr(node).first.get();
}

template<typename Node>
llvm::Value* LLVMCompiler::compile_expr_ptr(const std::unique_ptr<Node>& node) {
    return compile_expr(node).second.get();
}

template<typename Node>
std::pair<lazyValue<llvm::Value*>, lazyValue<llvm::Value*>> LLVMCompiler::compile_expr(const std::unique_ptr<Node>& node) {
    node->accept(*this);
    auto ret = expressions.top();
    expressions.pop();
    return ret;
}   

#endif
