#include "Expressions.hpp"

#include "Generator.hpp"

namespace yat
{
    Expression const& BinaryExpression::lhs() const
    {
        return operands.front();
    }

    Expression const& BinaryExpression::rhs() const
    {
        return operands.back();
    }

    Type BinaryExpression::type(Scope const& table) const
    {
        Expression const& lhs = operands.front();
        Expression const& rhs = operands.back();

        Type a = lhs.type(table);
        Type b = rhs.type(table);
        if (a == b)
        {
            switch (op)
            {
            case yat::Operator::Equals:
            case yat::Operator::NotEquals:
                return Type::INT;
            default:
                return a;
            }
        }

        if ((a == Type::INT && b == Type::FLOAT) || (a == Type::FLOAT && b == Type::INT))
        {
            switch (op)
            {
            case yat::Operator::Equals:
            case yat::Operator::NotEquals:
                return Type::INT;
            default:
                return Type::FLOAT;
            }
        }

        return Type::ERROR;
    }

    llvm::Value * BinaryExpression::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        Expression const& lhs_expr = lhs();
        Expression const& rhs_expr = rhs();

        Type lhs_type = lhs_expr.type(scope);
        Type rhs_type = lhs_expr.type(scope);

        llvm::Value * lhs_value = lhs_expr.compile(scope, block);
        llvm::Value * rhs_value = rhs_expr.compile(scope, block);

        Type expr_type = Type::INT;
        if (lhs_type == Type::FLOAT || rhs_type == Type::FLOAT)
        {
            expr_type = Type::FLOAT;
        }

        if (lhs_type == Type::INT && rhs_type == Type::FLOAT)
        {
            lhs_value = llvm::CastInst::Create(
                llvm::CastInst::CastOps::SIToFP,
                lhs_value,
                llvm::Type::getFloatTy(GlobalContext),
                "",
                block
            );
        }

        if (lhs_type == Type::FLOAT && rhs_type == Type::INT)
        {
            rhs_value = llvm::CastInst::Create(
                llvm::CastInst::CastOps::SIToFP,
                rhs_value,
                llvm::Type::getFloatTy(GlobalContext),
                "",
                block
            );
        }

#define RETURN_OP(op) \
    return llvm::BinaryOperator::Create( \
        op, \
        lhs_value, \
        rhs_value, \
        "", \
        block \
    )

#define CAST_TO_BOOL(value) \
    llvm::BinaryOperator::CreateAnd(value, llvm::ConstantInt::get(GlobalContext, llvm::APInt(32, 1, true)), "", block)

#define CAST_TO_INT(value) \
    CAST_TO_BOOL(llvm::CastInst::CreateIntegerCast(value, llvm::Type::getInt32Ty(GlobalContext), true, "", block))

#define RETURN_FCMP(op) \
    return CAST_TO_INT( \
        llvm::FCmpInst::Create( \
            llvm::FCmpInst::OtherOps::FCmp, \
            op, \
            lhs_value, \
            rhs_value, \
            "", \
            block \
        ) \
    )

#define RETURN_ICMP(op) \
    return CAST_TO_INT( \
        llvm::FCmpInst::Create( \
            llvm::FCmpInst::OtherOps::ICmp, \
            op, \
            lhs_value, \
            rhs_value, \
            "", \
            block \
        ) \
    )

        if (expr_type == Type::FLOAT)
        {
            switch (op)
            {
            case Operator::Equals: RETURN_FCMP(llvm::FCmpInst::Predicate::FCMP_OEQ);
            case Operator::NotEquals: RETURN_FCMP(llvm::FCmpInst::Predicate::FCMP_ONE);
            case Operator::Add: RETURN_OP(llvm::Instruction::BinaryOps::FAdd);
            case Operator::Sub: RETURN_OP(llvm::Instruction::BinaryOps::FSub);
            case Operator::Mul: RETURN_OP(llvm::Instruction::BinaryOps::FMul);
            case Operator::Div: RETURN_OP(llvm::Instruction::BinaryOps::FDiv);
            }
        }
        else if (expr_type == Type::INT)
        {
            switch (op)
            {
            case Operator::Equals: RETURN_ICMP(llvm::CmpInst::Predicate::ICMP_EQ);
            case Operator::NotEquals: RETURN_ICMP(llvm::CmpInst::Predicate::ICMP_NE);
            case Operator::Add: RETURN_OP(llvm::Instruction::BinaryOps::Add);
            case Operator::Sub: RETURN_OP(llvm::Instruction::BinaryOps::Sub);
            case Operator::Mul: RETURN_OP(llvm::Instruction::BinaryOps::Mul);
            case Operator::Div: RETURN_OP(llvm::Instruction::BinaryOps::SDiv);
            case Operator::And: RETURN_OP(llvm::Instruction::BinaryOps::And);
            case Operator::Or:  RETURN_OP(llvm::Instruction::BinaryOps::Or);
            }
        }
        else
        {
            return nullptr;
        }

#undef RETURN_OP
#undef RETURN_FCMP
#undef RETURN_ICMP
    }

    BinaryExpression::BinaryExpression(Operator op, Expression lhs, Expression rhs) :
        op(op), operands({ std::move(lhs), std::move(rhs) })
    {
    }

    std::string BinaryExpression::to_string() const
    {
        std::string op_text;

        switch (op)
        {
        case Operator::Add:
            op_text = "+";
            break;
        case Operator::Sub:
            op_text = "-";
            break;
        case Operator::Mul:
            op_text = "*";
            break;
        case Operator::Div:
            op_text = "/";
            break;
        }

        return lhs().to_string() + " " + op_text + " " + rhs().to_string();
    }

    Type IdentifierLiteral::type(Scope const& scope) const
    {
        return scope.variable(value)->type;
    }

    IdentifierLiteral::IdentifierLiteral(std::string value) :
        value(std::move(value))
    {
    }

    llvm::Value * IdentifierLiteral::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        return new llvm::LoadInst(scope.variable(value)->value, "", block);
    }

    std::string IdentifierLiteral::to_string() const
    {
        return value;
    }

    Type StringLiteral::type(Scope const&) const
    {
        return Type::STRING;
    }

    StringLiteral::StringLiteral(std::string value) :
        value(std::move(value))
    {
    }

    std::string StringLiteral::to_string() const
    {
        return "\"" + value + "\"";
    }

    Type FloatLiteral::type(Scope const&) const
    {
        return Type::FLOAT;
    }

    FloatLiteral::FloatLiteral(float value) :
        value(value)
    {
    }

    llvm::Value * FloatLiteral::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        return llvm::ConstantFP::get(GlobalContext, llvm::APFloat(value));
    }

    std::string FloatLiteral::to_string() const
    {
        return std::to_string(value);
    }

    Type IntLiteral::type(Scope const&) const
    {
        return Type::INT;
    }

    IntLiteral::IntLiteral(int value) :
        value(value)
    {
    }

    llvm::Value * IntLiteral::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        return llvm::ConstantInt::get(GlobalContext, llvm::APInt(32, value, true));
    }

    std::string IntLiteral::to_string() const
    {
        return std::to_string(value);
    }

    Type Expression::type(Scope const& table) const
    {
        return std::visit(
            [&table](auto && a) { return a.type(table); },
            value
        );
    }

    llvm::Value * Expression::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        return std::visit(
            [&](auto && a) { return a.compile(scope, block); },
            value
        );
    }

    std::string Expression::to_string() const
    {
        return std::visit(
            [](auto && a) { return a.to_string(); },
            value
        );
    }
    
    Cast::Cast(Expression expression) :
        expression({ expression })
    {
    }

    Type Cast::type(Scope const & scope) const
    {
        return Type::INT;
    }

    llvm::Value * Cast::compile(Scope const& scope, llvm::BasicBlock * block) const
    {
        Expression const& expr = expression.back();

        llvm::Value * value = expr.compile(scope, block);

        Type expr_type = expr.type(scope);
        if (expr_type == Type::FLOAT)
        {
            value = llvm::CastInst::Create(
                llvm::CastInst::CastOps::FPToSI,
                value,
                llvm::Type::getInt32Ty(GlobalContext),
                "",
                block
            );
        }

        return value;
    }
    
    std::string Cast::to_string() const
    {
        return "[" + expression.back().to_string() + "]";
    }
}