#include "semantic.hpp" 

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
            require(SemanticAnalyser::ExprType::IntPointer, SemanticAnalyser::ExprType::StringReference);
            yield(SemanticAnalyser::ExprType::Int, pos);
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
        analyse(arg);
        require(from_builtin_type(param_it->second));
        std::advance(param_it, 1);
    }

    yield(from_builtin_type(func.return_type), expr.position());
}

SemanticAnalyser::ExprType SemanticAnalyser::from_builtin_type(BuiltinType type) {
    switch (type) {
        case BuiltinType::Int: return SemanticAnalyser::ExprType::IntReference;
        case BuiltinType::String: return SemanticAnalyser::ExprType::StringReference;
        case BuiltinType::IntPointer: return SemanticAnalyser::ExprType::IntPointerReference;
    }
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
}

void SemanticAnalyser::visit(const FunctionDecl& func) {
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
    analyse(func.block);
    leave();
}

void SemanticAnalyser::visit(const VariableDecl& stmt) {
    for (const auto &var : stmt.var_decls) {
        declare_var(var);
        if (var.initial_value) {
            analyse(*var.initial_value);
        }
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
}

void SemanticAnalyser::visit(const ReturnStatement& stmt) {
    analyse(stmt.expr);
    require(SemanticAnalyser::ExprType::Int, 
            SemanticAnalyser::ExprType::IntReference, 
            SemanticAnalyser::ExprType::String, 
            SemanticAnalyser::ExprType::StringReference, 
            SemanticAnalyser::ExprType::IntPointer, 
            SemanticAnalyser::ExprType::IntPointerReference);
}

void SemanticAnalyser::visit(const ExpressionStatement& stmt) {
    analyse(stmt.expr);
    ignore();
}

void SemanticAnalyser::visit(const IfStatement& stmt) {
    for (const auto & conditional_block : stmt.blocks) {
        analyse(conditional_block.first);
        require(SemanticAnalyser::ExprType::Bool, SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);
        
        analyse(conditional_block.second);
    }
    if (stmt.else_statement) {
        analyse(*stmt.else_statement);
    }
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
}

void SemanticAnalyser::visit(const WhileStatement& stmt) {
    analyse(stmt.condition);
    require(SemanticAnalyser::ExprType::Bool, SemanticAnalyser::ExprType::Int, SemanticAnalyser::ExprType::IntReference);

    analyse(stmt.block);
}

void SemanticAnalyser::visit(const Program& program) {
    enter();
    for (const auto & var : program.global_vars) {
        analyse(var);
    }
    for (const auto & function : program.functions) {
        analyse(function);
    }
    leave();
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

void SemanticAnalyser::report_argument_number_mismatch(std::size_t expected, std::size_t got, const Position& position) const {
    throw SemanticException {
            concat(position_in_file(position), L"\n In \n",
                source->get_lines(position.line_number, position.line_number+1), L"\n",
                error_marker(position), L"\n\n",
                L"Wrong number of arguments, expected `", std::to_wstring(expected), L"` but got`", std::to_wstring(got),L"`."    
            )
    };
}


std::wstring SemanticAnalyser::error_marker(const Position& pos) {
    std::wstring ret = L"\033[1;32m";
    for (std::size_t i=1; i < pos.column_number-1; ++i) {
        ret += L"-";
    }
    ret += L"\033[1;31m^\033[0m";
    return ret;
}

const std::unordered_set<std::wstring> SemanticAnalyser::reserved_words = {
	L"int", 
	L"string"
};
