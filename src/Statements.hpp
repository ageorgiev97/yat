#pragma once

#include <string>
#include <vector>
#include <variant>

#include "Llvm.hpp"

#include "Expressions.hpp"

namespace yat
{
    class Statement;

    struct DeclarationStatement
    {
        Type type;
        std::string name;

        DeclarationStatement() = default;
        DeclarationStatement(Type type, std::string name);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct AssignmentStatement
    {
        std::string name;
        Expression expression;

        AssignmentStatement() = default;
        AssignmentStatement(std::string name, Expression expression);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct DeclarationAssignment
    {
        DeclarationStatement declaration;
        AssignmentStatement assignment;

        DeclarationAssignment() = default;
        DeclarationAssignment(DeclarationStatement declaration, AssignmentStatement assignment);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct ScopeStatement
    {
        mutable std::vector<Block> statements;

        ScopeStatement() = default;
        ScopeStatement(std::vector<Block> statements);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct ExpressionStatement
    {
        Expression expression;

        ExpressionStatement() = default;
        ExpressionStatement(Expression expression);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct ReturnExpression
    {
        Expression expression;

        ReturnExpression() = default;
        ReturnExpression(Expression expression);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct ReturnVoid
    {
        ReturnVoid() = default;

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct DefaultBlockEnd
    {
        DefaultBlockEnd() = default;

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct IfStatement
    {
        Expression expression;

        std::vector<Statement> operands;

        IfStatement() = default;
        IfStatement(Expression expression, std::vector<Statement> operands);

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };

    struct Statement
    {
        std::variant<
            DeclarationStatement,
            AssignmentStatement,
            DeclarationAssignment,
            ScopeStatement,
            ExpressionStatement,
            ReturnExpression,
            IfStatement,
            ReturnVoid,
            DefaultBlockEnd
        > value;

        Statement() = default;

        template <typename T>
        Statement(T const& value) :
            value(value)
        {
        }

        Statement(Statement const& other) = default;
        Statement(Statement && other) = default;

        Statement & operator=(Statement const& other) = default;
        Statement & operator=(Statement && other) = default;

        void compile(Scope & scope, llvm::Function * function, llvm::BasicBlock * block, llvm::BasicBlock * next) const;
    };
}