#pragma once

#include <any>
#include <variant>
#include <string>
#include <vector>
#include <type_traits>

#include "Llvm.hpp"

#include "Type.hpp"
#include "Scope.hpp"

namespace yat
{
    struct Expression;
    struct Block;

    struct IntLiteral
    {
        int value;

        Type type(Scope const&) const;

        IntLiteral() = default;
        IntLiteral(int value);

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        std::string to_string() const;
    };

    struct FloatLiteral
    {
        float value;

        Type type(Scope const&) const;

        FloatLiteral() = default;
        FloatLiteral(float value);

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        std::string to_string() const;
    };

    struct StringLiteral
    {
        std::string value;

        Type type(Scope const&) const;

        StringLiteral() = default;
        StringLiteral(std::string value);

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const { return nullptr; }

        std::string to_string() const;
    };

    struct IdentifierLiteral
    {
        std::string value;

        // Symbol table
        Type type(Scope const& scope) const;

        IdentifierLiteral() = default;
        IdentifierLiteral(std::string value);

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        std::string to_string() const;
    };

    struct Cast
    {
        std::vector<Expression> expression;

        Cast() = default;
        Cast(Expression expression);

        Type type(Scope const& scope) const;

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        std::string to_string() const;
    };

    enum class Operator
    {
        Add,
        Sub,
        Mul,
        Div,
        And,
        Or,
        Equals,
        NotEquals
        //...
    };

    struct BinaryExpression
    {
        std::vector<Expression> operands;

        Operator op;

        Expression const& lhs() const;
        Expression const& rhs() const;

        Type type(Scope const& table) const;

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        BinaryExpression() = default;
        BinaryExpression(Operator op, Expression lhs, Expression rhs);

        std::string to_string() const;
    };

    struct Expression
    {
        std::variant <
            IntLiteral,
            FloatLiteral,
            StringLiteral,
            IdentifierLiteral,
            BinaryExpression,
            Cast
        > value;

        Expression() = default;

        template <typename T>
        Expression(T const& value) :
            value(value)
        {
        }

        Expression(Expression const& other) = default;
        Expression(Expression && other) = default;

        Expression & operator=(Expression const& other) = default;
        Expression & operator=(Expression && other) = default;

        Type type(Scope const& table) const;

        llvm::Value * compile(Scope const& scope, llvm::BasicBlock * block) const;

        std::string to_string() const;
    };
}