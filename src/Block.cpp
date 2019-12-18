#include "Block.hpp"

#include "Llvm.hpp"

#include "Generator.hpp"

namespace yat
{
    void Block::initialize(llvm::Function * f)
    {
        function = f;
        block = llvm::BasicBlock::Create(GlobalContext, "", function);
    }

    void Block::compile(Scope & scope, llvm::BasicBlock * next)
    {
        for (Statement const& statement : statements)
        {
            statement.compile(scope, function, block, next);
        }
    }
}