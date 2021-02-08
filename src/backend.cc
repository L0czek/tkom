#include "backend.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

llvm::LLVMContext LLVMCompiler::ctx;

std::unique_ptr<LLVMCompiler> compile(const std::unique_ptr<Program> &program, const std::string &target,
                                      const std::string &data_layout)
{
    auto compiler = std::make_unique<LLVMCompiler>(target, data_layout);
    program->accept(*compiler);
    return compiler;
}

LLVMCompiler::LLVMCompiler(const std::string &target, const std::string &data_layout)
    : module(std::make_unique<llvm::Module>("top", ctx)), builder(ctx), data_layout_str(data_layout),
      target_triple(target), data_layout(data_layout_str)
{
    module->setTargetTriple(target_triple);
    module->setDataLayout(data_layout);
}

void LLVMCompiler::save_ir(const std::string &path)
{
    std::error_code ec;
    llvm::raw_fd_ostream fd(path, ec, llvm::sys::fs::F_None);
    fd << *module;
}

void LLVMCompiler::save_bc(const std::string &path)
{
    std::error_code ec;
    llvm::raw_fd_ostream fd(path, ec, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(*module, fd);
}

void LLVMCompiler::print_ir()
{
    module->print(llvm::outs(), nullptr);
}

int LLVMCompiler::execute()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    std::string err;
    llvm::EngineBuilder engine_builder(std::move(module));
    engine_builder.setEngineKind(llvm::EngineKind::JIT).setErrorStr(&err);
    llvm::ExecutionEngine *execution_engine = engine_builder.create();
    if (!execution_engine) {
        report_jit_creation_error(err);
    }
    std::vector<llvm::GenericValue> noargs;
    return static_cast<int>(execution_engine->runFunction(entrypoint_function, noargs).IntVal.getLimitedValue());
}

void LLVMCompiler::declare_global_var(const std::wstring &name, BuiltinType type)
{
    auto llvm_type = from_builtin_type(type);
    auto var = new llvm::GlobalVariable(*module, llvm_type, false, llvm::GlobalValue::CommonLinkage,
                                        llvm::Constant::getNullValue(llvm_type));
    global_vars.insert(std::make_pair(name, LLVMCompiler::Variable{ llvm_type, var }));
}

void LLVMCompiler::declare_global_var(const std::unique_ptr<VariableDecl> &stmt)
{
    for (const auto &var : stmt->var_decls) {
        declare_global_var(var.name, var.type);
    }
}

llvm::Type *LLVMCompiler::from_builtin_type(BuiltinType type)
{
    switch (type) {
    case BuiltinType::Int:
        return builder.getInt32Ty();
    case BuiltinType::IntPointer:
        return llvm::Type::getInt32PtrTy(ctx);
    case BuiltinType::String:
        return llvm::Type::getInt32PtrTy(ctx);
    }
    throw CompilerException(L"Invalid type");
}

void LLVMCompiler::yield(lazyValue<llvm::Value *> value, lazyValue<llvm::Value *> address)
{
    expressions.push(std::make_pair(value, address));
}

void LLVMCompiler::visit(const UnaryExpression &expr)
{
    switch (expr.op) {
    case UnaryOperator::Minus:
        yield(builder.CreateNeg(compile_expr_val(expr.rhs)));
        break;
    case UnaryOperator::BooleanNeg:
    case UnaryOperator::Neg:
        yield(builder.CreateNot(compile_expr_val(expr.rhs)));
        break;
    case UnaryOperator::Addrof:
        yield(compile_expr_ptr(expr.rhs));
        break;
    case UnaryOperator::Deref:
        auto [value, address] = compile_expr(expr.rhs);
        auto lazy_value = lazyValue<llvm::Value *>([this, value]() { return builder.CreateLoad(value.get()); });
        yield(lazy_value, value);
        break;
    }
}

void LLVMCompiler::visit(const BinaryExpression &expr)
{
    auto lhs = compile_expr_val(expr.lhs);
    auto rhs = compile_expr_val(expr.rhs);

    switch (expr.op) {
    case BinaryOperator::Plus:
        yield(builder.CreateAdd(lhs, rhs));
        break;
    case BinaryOperator::Minus:
        yield(builder.CreateSub(lhs, rhs));
        break;
    case BinaryOperator::Multiply:
        yield(builder.CreateMul(lhs, rhs));
        break;
    case BinaryOperator::Divide:
        yield(builder.CreateSDiv(lhs, rhs));
        break;
    case BinaryOperator::Modulo:
        yield(builder.CreateSRem(lhs, rhs));
        break;
    case BinaryOperator::BooleanAnd:
    case BinaryOperator::And:
        yield(builder.CreateAnd(lhs, rhs));
        break;
    case BinaryOperator::Xor:
        yield(builder.CreateXor(lhs, rhs));
        break;
    case BinaryOperator::BooleanOr:
    case BinaryOperator::Or:
        yield(builder.CreateOr(lhs, rhs));
        break;
    case BinaryOperator::ShiftLeft:
        yield(builder.CreateShl(lhs, rhs));
        break;
    case BinaryOperator::ShiftRight:
        yield(builder.CreateAShr(lhs, rhs));
        break;
    case BinaryOperator::Less:
        yield(builder.CreateICmpSLT(lhs, rhs));
        break;
    case BinaryOperator::Greater:
        yield(builder.CreateICmpSGT(lhs, rhs));
        break;
    case BinaryOperator::LessEqual:
        yield(builder.CreateICmpSLE(lhs, rhs));
        break;
    case BinaryOperator::GreaterEqual:
        yield(builder.CreateICmpSGE(lhs, rhs));
        break;
    case BinaryOperator::Equal:
        yield(builder.CreateICmpEQ(lhs, rhs));
        break;
    case BinaryOperator::NotEqual:
        yield(builder.CreateICmpNE(lhs, rhs));
        break;
    }
}

void LLVMCompiler::visit(const IndexExpression &expr)
{
    auto ptr = compile_expr_val(expr.ptr);
    auto index = compile_expr_val(expr.index);
    auto address = builder.CreateGEP(ptr, index);
    auto lazy_value = lazyValue<llvm::Value *>(
        [this, address]() { return builder.CreateSExtOrTrunc(builder.CreateLoad(address), builder.getInt32Ty()); });
    yield(lazy_value, address);
}

llvm::Value *LLVMCompiler::convert_to_bool(llvm::Value *expr)
{
    if (expr->getType() != builder.getInt1Ty()) {
        return builder.CreateICmpNE(expr, builder.getInt32(0));
    } else {
        return expr;
    }
}

void LLVMCompiler::visit(const VariableRef &expr)
{
    auto address = get_variable_ptr(expr.var_name);
    auto lazy_value = lazyValue<llvm::Value *>([this, address]() { return builder.CreateLoad(address); });
    yield(lazy_value, address);
}

void LLVMCompiler::visit(const FunctionCall &expr)
{
    const auto &function = functions.at(expr.func_name);
    std::vector<llvm::Value *> values;
    for (const auto &argument : expr.arguments) {
        values.push_back(compile_expr_val(argument));
    }
    yield(builder.CreateCall(function.llvm_ptr, values));
}

void LLVMCompiler::visit(const IntConst &expr)
{
    yield(llvm::ConstantInt::get(builder.getInt32Ty(), expr.value));
}

void LLVMCompiler::visit(const StringConst &expr)
{
    const char *ptr = reinterpret_cast<const char *>(expr.value.c_str());
    std::size_t size = (expr.value.length() + 1) * sizeof(wchar_t);
    yield(builder.CreateBitCast(builder.CreateGlobalStringPtr(llvm::StringRef(ptr, size)),
                                llvm::Type::getInt32PtrTy(ctx)));
}

void LLVMCompiler::visit(const Block &block)
{
    enter();
    for (const auto &stmt : block.statements) {
        compile(stmt);
    }
    leave();
}

void LLVMCompiler::visit(const ExternFunctionDecl &decl)
{
    Function function;
    std::vector<llvm::Type *> parameter_types;
    for (const auto &param : decl.parameters) {
        function.parameters.push_back(from_builtin_type(param.type));
    }
    llvm::ArrayRef<llvm::Type *> params_ref{ function.parameters };
    function.type = llvm::FunctionType::get(from_builtin_type(decl.return_type), params_ref, false);
    function.llvm_ptr = llvm::Function::Create(function.type, llvm::Function::ExternalLinkage, "", *module);
    function.llvm_ptr->setCallingConv(llvm::CallingConv::C);
    std::string ascii_name(decl.func_name.begin(), decl.func_name.end());
    function.llvm_ptr->setName(ascii_name);
    functions.insert(std::make_pair(decl.func_name, std::move(function)));
}

void LLVMCompiler::visit(const FunctionDecl &decl)
{
    Function function;
    std::vector<llvm::Type *> parameter_types;
    for (const auto &param : decl.parameters) {
        function.parameters.push_back(from_builtin_type(param.type));
    }
    llvm::ArrayRef<llvm::Type *> params_ref{ function.parameters };
    function.type = llvm::FunctionType::get(from_builtin_type(decl.return_type), params_ref, false);
    function.llvm_ptr = llvm::Function::Create(function.type, llvm::Function::ExternalLinkage, "", *module);
    function.llvm_ptr->setCallingConv(llvm::CallingConv::C);
    function.llvm_ptr->arg_begin();
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "entry", function.llvm_ptr);
    builder.SetInsertPoint(entry);
    functions.insert(std::make_pair(decl.func_name, std::move(function)));
    enter();
    process_parameters(decl.parameters, function.llvm_ptr);
    current_function = function.llvm_ptr;
    compile(decl.block);
    leave();

    for (auto it = function.llvm_ptr->begin(); it != function.llvm_ptr->end(); ++it) {
        remove_dead_code(*it);
    }
}

void LLVMCompiler::process_parameters(const std::list<FunctionDecl::Parameter> &parameters, llvm::Function *function)
{
    auto param_it = function->arg_begin();
    for (const auto &param : parameters) {
        auto type = param_it->getType();
        auto ptr = builder.CreateAlloca(type);
        declare_variable(param.name, ptr, type);
        builder.CreateStore(param_it, ptr);
        std::advance(param_it, 1);
    }
}

void LLVMCompiler::enter()
{
    scopes.push_back({});
}

void LLVMCompiler::leave()
{
    scopes.pop_back();
}

void LLVMCompiler::declare_variable(const std::wstring &name, llvm::Value *ptr, llvm::Type *type)
{
    scopes.back().insert(std::make_pair(name, LLVMCompiler::Variable{ type, ptr }));
}

const LLVMCompiler::Variable &LLVMCompiler::find_variable(const std::wstring &name)
{
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        auto it = scope->find(name);
        if (it != scope->end()) {
            return it->second;
        }
    }
    return global_vars.find(name)->second;
    // after semantic analysys should never reach here
}

llvm::Value *LLVMCompiler::get_variable_ptr(const std::wstring &name)
{
    return find_variable(name).ptr;
}

llvm::Type *LLVMCompiler::get_variable_type(const std::wstring &name)
{
    return find_variable(name).type;
}

void LLVMCompiler::remove_dead_code(llvm::BasicBlock &block)
{
    for (auto it = block.begin(); it != block.end();) {
        if (it->isTerminator()) {
            it++;
            while (it != block.end()) {
                it = it->eraseFromParent();
            }
        } else {
            it++;
        }
    }
}

void LLVMCompiler::visit(const VariableDecl &stmt)
{
    for (const auto &var : stmt.var_decls) {
        auto type = from_builtin_type(var.type);
        auto ptr = builder.CreateAlloca(type);
        if (var.initial_value) {
            auto value = compile_expr_val(*var.initial_value);
            builder.CreateStore(value, ptr);
        }
        declare_variable(var.name, ptr, type);
    }
}

void LLVMCompiler::visit(const AssignmentStatement &stmt)
{
    auto value = compile_expr_val(stmt.parts.back());
    auto it = stmt.parts.cbegin();
    for (std::size_t i = 0; i < stmt.parts.size() - 1; ++i) {
        auto address = compile_expr_ptr(*it);
        builder.CreateStore(value, address);
        std::advance(it, 1);
    }
}

void LLVMCompiler::visit(const ReturnStatement &stmt)
{
    builder.CreateRet(compile_expr_val(stmt.expr));
}

void LLVMCompiler::visit(const ExpressionStatement &stmt)
{
    compile_expr_val(stmt.expr);
}

void LLVMCompiler::visit(const IfStatement &stmt)
{
    llvm::BasicBlock *after_if = llvm::BasicBlock::Create(ctx, "after_if", current_function);
    llvm::BasicBlock *cond_true;
    llvm::BasicBlock *cond_false;
    for (const auto &element : stmt.blocks) {
        const auto &[condition, block] = element;
        auto value = compile_expr_val(condition);
        cond_true = llvm::BasicBlock::Create(ctx, "cond_true", current_function);
        cond_false = llvm::BasicBlock::Create(ctx, "cond_false", current_function);
        builder.CreateCondBr(convert_to_bool(value), cond_true, cond_false);
        builder.SetInsertPoint(cond_true);
        compile(block);
        builder.CreateBr(after_if);
        builder.SetInsertPoint(cond_false);
    }
    if (stmt.else_statement) {
        compile(*stmt.else_statement);
    }
    builder.CreateBr(after_if);
    builder.SetInsertPoint(after_if);
}

void LLVMCompiler::visit(const ForStatement &stmt)
{
    auto start = compile_expr_val(stmt.start);
    auto end = compile_expr_val(stmt.end);
    llvm::Value *increase;
    if (stmt.increase) {
        increase = compile_expr_val(*stmt.increase);
    } else {
        increase = llvm::ConstantInt::get(builder.getInt32Ty(), 1);
    }
    enter();
    auto ptr = builder.CreateAlloca(builder.getInt32Ty());
    builder.CreateStore(start, ptr);
    declare_variable(stmt.loop_variable, ptr, builder.getInt32Ty());
    llvm::BasicBlock *loop_condition = llvm::BasicBlock::Create(ctx, "loop_condition", current_function);
    llvm::BasicBlock *loop_body = llvm::BasicBlock::Create(ctx, "loop_body", current_function);
    llvm::BasicBlock *after_loop = llvm::BasicBlock::Create(ctx, "after_loop", current_function);
    builder.CreateBr(loop_condition);
    builder.SetInsertPoint(loop_condition);
    auto iterator = builder.CreateLoad(ptr);
    auto condition = builder.CreateICmpSLT(iterator, end);
    builder.CreateCondBr(convert_to_bool(condition), loop_body, after_loop);
    builder.SetInsertPoint(loop_body);
    compile(stmt.block);
    iterator = builder.CreateLoad(ptr);
    auto new_iterator = builder.CreateAdd(iterator, increase);
    builder.CreateStore(new_iterator, ptr);
    builder.CreateBr(loop_condition);
    builder.SetInsertPoint(after_loop);
    leave();
}

void LLVMCompiler::visit(const WhileStatement &stmt)
{
    llvm::BasicBlock *loop_condition = llvm::BasicBlock::Create(ctx, "loop_condition", current_function);
    llvm::BasicBlock *loop_body = llvm::BasicBlock::Create(ctx, "loop_body", current_function);
    llvm::BasicBlock *after_loop = llvm::BasicBlock::Create(ctx, "after_loop", current_function);
    builder.CreateBr(loop_condition);
    builder.SetInsertPoint(loop_condition);
    auto condition = compile_expr_val(stmt.condition);
    builder.CreateCondBr(convert_to_bool(condition), loop_body, after_loop);
    builder.SetInsertPoint(loop_body);
    compile(stmt.block);
    builder.CreateBr(loop_condition);
    builder.SetInsertPoint(after_loop);
}

void LLVMCompiler::compile_entrypoint(const std::list<std::unique_ptr<VariableDecl> > &global_vars_decl)
{
    llvm::FunctionType *type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "main", *module);
    entrypoint_function = function;
    llvm::BasicBlock *entrypoint = llvm::BasicBlock::Create(ctx, "__entrypoint__", function);
    builder.SetInsertPoint(entrypoint);
    for (const auto &vars : global_vars_decl) {
        initialize_variables(vars);
    }
    auto main_it = functions.find(L"main");
    if (main_it == functions.end()) {
        report_undefined_main();
    }
    builder.CreateRet(builder.CreateCall(main_it->second.llvm_ptr));
}

void LLVMCompiler::initialize_variables(const std::unique_ptr<VariableDecl> &decl)
{
    for (const auto &var : decl->var_decls) {
        if (var.initial_value) {
            auto value = compile_expr_val(*var.initial_value);
            auto address = get_variable_ptr(var.name);
            builder.CreateStore(value, address);
        }
    }
}

void LLVMCompiler::visit(const Program &program)
{
    for (const auto &extern_func : program.externs) {
        compile(extern_func);
    }
    for (const auto &stmt : program.global_vars) {
        declare_global_var(stmt);
    }
    for (const auto &function : program.functions) {
        compile(function);
    }
    compile_entrypoint(program.global_vars);
    llvm::verifyModule(*module, &llvm::errs());
    optimize();
}

void LLVMCompiler::optimize()
{
    auto FPM = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());
    FPM->add(llvm::createInstructionCombiningPass());
    FPM->add(llvm::createReassociatePass());
    FPM->add(llvm::createGVNPass());
    FPM->add(llvm::createCFGSimplificationPass());
    FPM->doInitialization();

    for (auto &function : functions) {
        FPM->run(*function.second.llvm_ptr);
    }
}

void LLVMCompiler::report_undefined_main()
{
    throw CompilerException{ L"Undefined reference to main function" };
}

void LLVMCompiler::report_jit_creation_error(const std::string &msg)
{
    std::wstring wstr(msg.begin(), msg.end());
    throw CompilerException{ concat(L"Cannot create LLVM's JIT, reason: ", wstr) };
}

std::string default_data_layout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                                  "i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-"
                                  "a0:0:64-s0:64:64-f80:128:128";

std::string default_target_triple = "x86_64-unknown-linux-gnu";
