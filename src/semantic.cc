#include "semantic.hpp" 
#include <cassert>

#define ASSERT_EMPTY_STACK assert(stack.empty())
#define ASSERT_EMPTY_SCOPE assert(scopes.empty())
#define ASSERT_EMPTY_RET_STACK assert(has_return.empty())

void analyse(const std::unique_ptr<Program>& program, std::unique_ptr<Source> source) {
    SemanticAnalyser analyser{std::move(source)};
    program->accept(analyser);
}

void SemanticAnalyser::yield(SemanticAnalyser::ExprType type, const Position& pos) {
	stack.push(std::make_pair(type, pos));
}

void SemanticAnalyser::ignore() {
    stack.pop();
}

void SemanticAnalyser::enter() {
	scopes.push_back({});
}

void SemanticAnalyser::leave() {
    scopes.pop_back();
}

SemanticAnalyser::ExprType SemanticAnalyser::pop() {
    auto ret = stack.top().first;
    stack.pop();
    return ret;
}
bool SemanticAnalyser::is_one_of(ExprType allowed) {
    return stack.top().first == allowed;
}

BuiltinType SemanticAnalyser::get_var(const VariableRef& var) const {
    check_id(var.var_name, var.position());
    for (auto it=scopes.crbegin(); it != scopes.crend(); ++it) {
        if (is_in_scope(var.var_name, *it)) {
            return var_from_scope(var.var_name, *it);
        }
    }
    report_undefined_variable(var.var_name, var.position());
}

BuiltinType SemanticAnalyser::var_from_scope(
        const std::wstring& name, 
        const std::unordered_map<std::wstring, BuiltinType>& scope
    ) const {
    return scope.at(name);
}

void SemanticAnalyser::declare_var(const VariableDecl::SingleVarDecl& var) {
    check_id(var.name, var.position());
    if (is_in_scope(var.name, scopes.back())) {
        report_variable_redeclaration(var.name, var.position());
    }
    scopes.back().insert(std::make_pair(var.name, var.type));
}

void SemanticAnalyser::check_id(const std::wstring& name, const Position& position) const {
    if (reserved_words.find(name) != reserved_words.end()) {
        report_reserved_word(name, position);
    }
}
  
bool SemanticAnalyser::is_in_scope(const std::wstring& name, const std::unordered_map<std::wstring, BuiltinType>& variables) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return true;
    } else {
        return false;
    }
}

void SemanticAnalyser::visit(const UnaryExpression& expr) {
    analyse(expr.rhs);
    auto pos = expr.position();

    switch (expr.op) {
        case UnaryOperator::Minus:      
        case UnaryOperator::Neg:
            require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
            yield(SemanticAnalyser::ExprType::Int, pos);
            break;
        case UnaryOperator::Addrof: 
            require(SemanticAnalyser::ExprType::IntReference);
            yield(SemanticAnalyser::ExprType::IntPointer, pos);
            break;
        case UnaryOperator::Deref:      
            require(SemanticAnalyser::ExprType::IntPointer, 
                    SemanticAnalyser::ExprType::StringReference,
                    SemanticAnalyser::ExprType::IntPointerReference);
            yield(SemanticAnalyser::ExprType::IntReference, pos);
            break;
        case UnaryOperator::BooleanNeg:
            require(SemanticAnalyser::ExprType::Bool);
            yield(SemanticAnalyser::ExprType::Bool, pos);
            break;
    }
}

void SemanticAnalyser::visit(const BinaryExpression& expr) {
    analyse(expr.lhs);
    analyse(expr.rhs);

    const auto & pos = expr.position();
    switch (expr.op) {
        case BinaryOperator::Plus:
        case BinaryOperator::Minus:
        case BinaryOperator::Multiply:
        case BinaryOperator::Divide:
        case BinaryOperator::Modulo:
        case BinaryOperator::And:
        case BinaryOperator::Xor:
        case BinaryOperator::Or:
        case BinaryOperator::ShiftLeft:
        case BinaryOperator::ShiftRight:
            require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
            require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
            yield(SemanticAnalyser::ExprType::Int, pos);
            break;
        case BinaryOperator::Less:
        case BinaryOperator::Greater:
        case BinaryOperator::LessEqual:
        case BinaryOperator::GreaterEqual:
        case BinaryOperator::Equal:
        case BinaryOperator::NotEqual:
            require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
            require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
            yield(SemanticAnalyser::ExprType::Bool, pos);
            break;
        case BinaryOperator::BooleanAnd:
        case BinaryOperator::BooleanOr:
            require(SemanticAnalyser::ExprType::Bool);
            require(SemanticAnalyser::ExprType::Bool);
            yield(SemanticAnalyser::ExprType::Bool, pos);
            break;
    }
}

void SemanticAnalyser::visit(const IndexExpression& expr) {
    analyse(expr.ptr);
    require(SemanticAnalyser::ExprType::IntPointer, 
            SemanticAnalyser::ExprType::IntPointerReference, 
            SemanticAnalyser::ExprType::StringReference);

    analyse(expr.index);
    require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);

    yield(SemanticAnalyser::ExprType::IntReference, expr.position());
}

void SemanticAnalyser::visit(const VariableRef& var) {
    const auto type = get_var(var); 
    const auto & pos = var.position();
    
    yield(from_builtin_type(type), pos);
}

const SemanticAnalyser::Function& SemanticAnalyser::function_from_name(const std::wstring& name, const Position& pos) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second;
    } else {
        report_undefined_function(name, pos);
    }
}

void SemanticAnalyser::visit(const FunctionCall& expr) {
    const auto & position = expr.position();
    check_id(expr.func_name, position);
    const auto & func = function_from_name(expr.func_name, position);
    
    std::size_t expected = func.parameters.size();
    std::size_t got = expr.arguments.size();
    if (expected != got) {
        report_argument_number_mismatch(expected, got, expr.position());
    }

    auto param_it = func.parameters.cbegin();
    for (const auto &arg : expr.arguments) {
        check_assignable_by(param_it->second, arg);
        std::advance(param_it, 1);
    }

    yield(from_builtin_type_value(func.return_type), expr.position());
}

SemanticAnalyser::ExprType SemanticAnalyser::from_builtin_type_value(BuiltinType type) {
    switch (type) {
        case BuiltinType::Int: return SemanticAnalyser::ExprType::Int;
        case BuiltinType::String: return SemanticAnalyser::ExprType::String;
        case BuiltinType::IntPointer: return SemanticAnalyser::ExprType::IntPointer;
    }
    throw SemanticException(L"Invalid type");
}

SemanticAnalyser::ExprType SemanticAnalyser::from_builtin_type(BuiltinType type) {
    switch (type) {
        case BuiltinType::Int: return SemanticAnalyser::ExprType::IntReference;
        case BuiltinType::String: return SemanticAnalyser::ExprType::StringReference;
        case BuiltinType::IntPointer: return SemanticAnalyser::ExprType::IntPointerReference;
    }
    throw SemanticException(L"Invalid type");
}

void SemanticAnalyser::visit(const IntConst& expr) {
    yield(SemanticAnalyser::ExprType::Int, expr.position());
}

void SemanticAnalyser::visit(const StringConst& expr) {
    yield(SemanticAnalyser::ExprType::String, expr.position());
}

void SemanticAnalyser::visit(const Block& block) {
    enter();
    for (const auto & stmt : block.statements) {
        analyse(stmt);
    }
    leave();
    yield_return_one(block.statements.size());
}

void SemanticAnalyser::visit(const ExternFunctionDecl& func) {
    check_id(func.func_name, func.position());
    if (functions.find(func.func_name) != functions.end()) {
        report_function_redeclaration(func.func_name, func.position());
    }

    enter();
    for (const auto & param : func.parameters) {
        if (is_in_scope(param.name, scopes.back())) {
            report_parameter_redeclaration(param.name, param.position());
        }
        scopes.back().insert(std::make_pair(param.name, param.type));
    }
    Function declaration;
    declaration.return_type = func.return_type;
    for (const auto & param : func.parameters) {
        declaration.parameters.push_back(std::make_pair(param.name, param.type));
    }
    functions.insert(std::make_pair(func.func_name, declaration)); // To enable recursion
    leave();

    ASSERT_EMPTY_RET_STACK;
}

void SemanticAnalyser::check_main_function(const FunctionDecl& decl) {
    if (decl.func_name == L"main") {
        if (decl.parameters.size() != 0) {
            report_main_bad_params(decl.parameters.front().position());
        }
        if (decl.return_type != BuiltinType::Int) {
            report_main_bad_return_type(decl.position());
        }
    }
}

void SemanticAnalyser::visit(const FunctionDecl& func) {
    check_id(func.func_name, func.position());
    check_main_function(func);
    if (functions.find(func.func_name) != functions.end()) {
        report_function_redeclaration(func.func_name, func.position());
    }

    enter();
    for (const auto & param : func.parameters) {
        if (is_in_scope(param.name, scopes.back())) {
            report_parameter_redeclaration(param.name, param.position());
        }
        scopes.back().insert(std::make_pair(param.name, param.type));
    }
    Function declaration;
    declaration.return_type = func.return_type;
    for (const auto & param : func.parameters) {
        declaration.parameters.push_back(std::make_pair(param.name, param.type));
    }
    functions.insert(std::make_pair(func.func_name, declaration)); // To enable recursion
    current_func_ret_type = func.return_type;
    analyse(func.block);
    assert_returns(func.position());
    leave();

    ASSERT_EMPTY_RET_STACK;
}

void SemanticAnalyser::visit(const VariableDecl& stmt) {
    for (const auto &var : stmt.var_decls) {
        declare_var(var);
        if (var.initial_value) {
           check_assignable_by(var.type, *var.initial_value);         
        }
    }
    yield_no_return();
}

void SemanticAnalyser::check_assignable_by(BuiltinType type, const std::unique_ptr<Expression>& expr) {
    analyse(expr);

    switch (type) {
        case BuiltinType::Int: 
            require(SemanticAnalyser::ExprType::Int, 
                    SemanticAnalyser::ExprType::IntReference); 
            break;
        case BuiltinType::IntPointer: 
            require(SemanticAnalyser::ExprType::IntPointer, 
                    SemanticAnalyser::ExprType::IntPointerReference); 
            break;
        case BuiltinType::String: 
            require(SemanticAnalyser::ExprType::String, 
                    SemanticAnalyser::ExprType::StringReference); 
            break;
    }
}

void SemanticAnalyser::check_assignable_by(const std::unique_ptr<Expression>& expr, SemanticAnalyser::ExprType rhs) {
    analyse(expr);
    switch (rhs) {
        case SemanticAnalyser::ExprType::Int:
        case SemanticAnalyser::ExprType::IntReference:
            require(SemanticAnalyser::ExprType::IntReference);
            break;
        case SemanticAnalyser::ExprType::String:
        case SemanticAnalyser::ExprType::StringReference:
            require(SemanticAnalyser::ExprType::StringReference);
            break;
        case SemanticAnalyser::ExprType::IntPointer:
        case SemanticAnalyser::ExprType::IntPointerReference:
            require(SemanticAnalyser::ExprType::IntPointerReference);
            break;
        default:
            throw SemanticException(L"Invalid assignment, this should not happend");
    }
}

void SemanticAnalyser::visit(const AssignmentStatement& stmt) {
    analyse(stmt.parts.back());
    const auto value_type = pop();
    auto it = stmt.parts.cbegin();
    for (std::size_t i=0; i < stmt.parts.size()-1; ++i) {
        check_assignable_by(*it, value_type);
        std::advance(it, 1);
    }
    yield_no_return();

    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const ReturnStatement& stmt) {
    check_assignable_by(current_func_ret_type, stmt.expr);
    yield_return();
    
    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const ExpressionStatement& stmt) {
    analyse(stmt.expr);
    ignore();
    yield_no_return();

    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const IfStatement& stmt) {
    for (const auto & conditional_block : stmt.blocks) {
        analyse(conditional_block.first);
        require(SemanticAnalyser::ExprType::Bool, SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
        
        analyse(conditional_block.second);
    }
    if (stmt.else_statement) {
        analyse(*stmt.else_statement);
        yield_return_all(stmt.blocks.size()+1); 
    } else {
        ignore_return(stmt.blocks.size());
        yield_no_return();
    }
    

    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const ForStatement& stmt) {
    enter();

    analyse(stmt.start);
    require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
    analyse(stmt.end);
    require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
    if (stmt.increase) {
        analyse(*stmt.increase);
        require(SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
    }
    check_id(stmt.loop_variable, stmt.loop_variable_pos);
    scopes.back().insert(std::make_pair(stmt.loop_variable, BuiltinType::Int));
    analyse(stmt.block);
    leave();

    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const WhileStatement& stmt) {
    analyse(stmt.condition);
    require(SemanticAnalyser::ExprType::Bool, SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);

    analyse(stmt.block);
    
    ASSERT_EMPTY_STACK;
}

void SemanticAnalyser::visit(const Program& program) {
    enter();
    for (const auto & extern_func : program.externs) {
        analyse(extern_func);
    }
    for (const auto & var : program.global_vars) {
        analyse(var);
    }
    ignore_return(program.global_vars.size());
    for (const auto & function : program.functions) {
        analyse(function);
    }
    leave();
    
    ASSERT_EMPTY_STACK;
    ASSERT_EMPTY_SCOPE;
}

std::wstring SemanticAnalyser::repr(SemanticAnalyser::ExprType type) {
    switch (type) {
        case SemanticAnalyser::ExprType::Int: return L"int";
        case SemanticAnalyser::ExprType::String: return L"string";
        case SemanticAnalyser::ExprType::IntPointer: return L"pointer to int";
        case SemanticAnalyser::ExprType::IntPointerReference: return L"reference to an int pointer";
        case SemanticAnalyser::ExprType::IntReference: return L"reference to a int variable";
        case SemanticAnalyser::ExprType::StringReference: return L"reference to a string variable";
        case SemanticAnalyser::ExprType::Bool: return L"boolean value";
    }
    throw SemanticException(L"Invalid type");
}

void SemanticAnalyser::yield_return() {
    has_return.push(true);
}

void SemanticAnalyser::yield_no_return() {
    has_return.push(false);
}

void SemanticAnalyser::ignore_return(std::size_t depth) {
    for (std::size_t i=0; i < depth; ++i) {
        has_return.pop();
    }
}

void SemanticAnalyser::yield_return_all(std::size_t depth) {
    if (depth == 0) {
        has_return.push(false);
    } else {
        bool conjunction = true;
        for (std::size_t i=0; i < depth; ++i) {
            conjunction &= has_return.top();
            has_return.pop();
        }
        has_return.push(conjunction);
    }
}

void SemanticAnalyser::yield_return_one(std::size_t depth) {
    bool disjunction = false;
    for (std::size_t i=0; i < depth; ++i) {
        disjunction |= has_return.top();
        has_return.pop();
    }
    has_return.push(disjunction);
}

void SemanticAnalyser::assert_returns(const Position& pos) {
    if (!has_return.top()) {
        report_no_return(pos);
    }
    has_return.pop();
}

void SemanticAnalyser::report_reserved_word(const std::wstring& word, const Position& position) const {
    throw SemanticException {
        concat(position_in_file(position), L"\n In \n", 
            source->get_lines(position.line_number, position.line_number+1), L"\n",
            error_marker(position), L"\n\n",
            L"Error word `", word, L"` is reserved and cannot by used as identifier."
            )
    };  
}

void SemanticAnalyser::report_undefined_variable(const std::wstring& name, const Position& position) const {
    throw SemanticException {
        concat(position_in_file(position), L"\n In \n", 
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Error cannot find variable of name`", name, L"` in scope."
                )
    };
}

void SemanticAnalyser::report_variable_redeclaration(const std::wstring& name, const Position& position) const {
    throw SemanticException {
        concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Error redclaration of variable `", name, L"`."    
            )
    };  
}

void SemanticAnalyser::report_function_redeclaration(const std::wstring& name, const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Error redclaration of function `", name, L"`."    
            )
    };
}

void SemanticAnalyser::report_parameter_redeclaration(const std::wstring& name, const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Error redclaration of parameter `", name, L"`."    
            )
    };

}

void SemanticAnalyser::report_undefined_function(const std::wstring& name, const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Error undefiend funtion with name = `", name, L"`."    
            )
    };
}

void SemanticAnalyser::report_no_return(const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Not all paths end with return statement."    
            )
    };
}

void SemanticAnalyser::report_argument_number_mismatch(std::size_t expected, std::size_t got, const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Wrong number of arguments, expected `", std::to_wstring(expected), L"` but got`", std::to_wstring(got),L"`."    
            )
    };
}

void SemanticAnalyser::report_main_bad_params(const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Main function should take no parameters (for now...) due to author laziness"    
            )
    };
}

void SemanticAnalyser::report_main_bad_return_type(const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Main function should return Int"    
            )
    };

}

const std::unordered_set<std::wstring> SemanticAnalyser::reserved_words = {
	L"int", 
	L"string"
};
