#include "Statements.hpp"

#include "Block.hpp"
#include "Generator.hpp"

namespace yat
{
    void Statement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        return std::visit(
            [&](auto && a) { return a.compile(scope, function, block, next); },
            value
        );
    }

    ExpressionStatement::ExpressionStatement(Expression expression) :
        expression(std::move(expression))
    {
    }

    void ExpressionStatement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        expression.compile(scope, block);
    }

    ScopeStatement::ScopeStatement(std::vector<Block> statements) :
        statements(std::move(statements))
    {
    }

    void ScopeStatement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        Scope new_scope(&scope);

        for (Block & statement : statements)
        {
            statement.initialize(function);
            statement.compile(new_scope, next);
        }

        llvm::BranchInst::Create(block, block);
    }
    
    DeclarationStatement::DeclarationStatement(Type type, std::string name) :
        type(type), name(std::move(name))
    {
    }

    void DeclarationStatement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        llvm::Type * llvm_type;

        if (type == Type::INT)
        {
            llvm_type = llvm::Type::getInt32Ty(GlobalContext);
        }
        else
        {
            llvm_type = llvm::Type::getFloatTy(GlobalContext);
        }

        llvm::Value * value = new llvm::AllocaInst(llvm_type, 0, "", block);

        scope.set_local(name, type, value);
    }

    AssignmentStatement::AssignmentStatement(std::string name, Expression expression) :
        name(std::move(name)), expression(std::move(expression))
    {
    }

    void AssignmentStatement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        new llvm::StoreInst(
            expression.compile(scope, block),
            scope.variable(name)->value,
            block
        );
    }

    DeclarationAssignment::DeclarationAssignment(DeclarationStatement declaration, AssignmentStatement assignment) :
        declaration(std::move(declaration)), assignment(std::move(assignment))
    {
    }

    void DeclarationAssignment::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        declaration.compile(scope, function, block, next);
        assignment.compile(scope, function, block, next);
    }

    ReturnExpression::ReturnExpression(Expression expression) :
        expression(std::move(expression))
    {
    }

    void ReturnExpression::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        llvm::ReturnInst::Create(GlobalContext, expression.compile(scope, block), block);
    }

    void ReturnVoid::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        llvm::ReturnInst::Create(GlobalContext, block);
    }

    void DefaultBlockEnd::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        if (next != nullptr)
        {
            llvm::BranchInst::Create(next, block);
        }
        else
        {
            llvm::ReturnInst::Create(GlobalContext, block);
        }
    }

    IfStatement::IfStatement(Expression expression, std::vector<Statement> operands) :
        expression(std::move(expression)), operands(std::move(operands))
    {
    }

    void IfStatement::compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const
    {
        llvm::BasicBlock * then_block = nullptr;
        llvm::BasicBlock * else_block = nullptr;
        
        then_block = llvm::BasicBlock::Create(GlobalContext, "then", function);
        Scope then_scope(&scope);

        operands.front().compile(then_scope, function, then_block, next);

        if (operands.size() == 2)
        {
            else_block = llvm::BasicBlock::Create(GlobalContext, "else", function);
            Scope else_scope(&scope);

            operands.back().compile(else_scope, function, then_block, next);
        }

        llvm::BranchInst::Create(then_block, else_block, expression.compile(scope, block), block);
    }
}