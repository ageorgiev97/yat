#pragma once

#include <vector>

#include "Llvm.hpp"

#include "Type.hpp"
#include "Block.hpp"

namespace yat
{
    struct Function
    {
        Type return_type;
        llvm::Function * function;

        std::vector<Block> blocks;

        Function() = default;
        Function(Type type, std::string const& name, std::vector<Block> blocks);
    };
}