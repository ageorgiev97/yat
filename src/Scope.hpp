#pragma once

#include <string>
#include <unordered_map>

#include "Llvm.hpp"

#include "Type.hpp"

namespace yat
{
    struct Variable
    {
        Type type;
        llvm::Value * value;

        Variable() = default;
        Variable(Type type, llvm::Value * value);
    };

    struct Scope
    {
    private:
        Scope * parent_;

        std::unordered_map<std::string, Variable> locals;

    public:
        Variable const* variable(std::string const& identifier) const;

        void set_local(std::string const& identifier, Type type, llvm::Value * value);

        Scope * parent() const;

        Scope(Scope * parent = nullptr);
    };
}