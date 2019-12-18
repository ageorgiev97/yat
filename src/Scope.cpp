#include "Scope.hpp"

namespace yat
{
    Variable::Variable(Type type, llvm::Value * value) :
        type(type), value(value)
    {
    }

    Variable const* Scope::variable(std::string const& identifier) const
    {
        auto it = locals.find(identifier);
        if (it != std::end(locals))
        {
            return &(it->second);
        }

        if (parent_ != nullptr)
        {
            return parent_->variable(identifier);
        }

        return nullptr;
    }

    void Scope::set_local(std::string const& identifier, Type type, llvm::Value * value)
    {
        locals[identifier] = Variable(type, value);
    }
    
    Scope * Scope::parent() const
    {
        return parent_;
    }

    Scope::Scope(Scope * parent) :
        parent_(parent)
    {
    }
}