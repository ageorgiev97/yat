#include "Function.hpp"

#include "Generator.hpp"

namespace yat
{
    Function::Function(Type type, std::string const& name, std::vector<Block> b) :
        return_type(type),
        blocks(std::move(b))
    {
        llvm::Type * t;
        if (return_type == Type::INT)
        {
            t = llvm::Type::getInt32Ty(GlobalContext);
        }
        else if (return_type == Type::FLOAT)
        {
            t = llvm::Type::getFloatTy(GlobalContext);
        }
        else
        {
            t = llvm::Type::getVoidTy(GlobalContext);
        }

        llvm::FunctionType * ftype = llvm::FunctionType::get(t, false);
        function = llvm::Function::Create(
            ftype,
            llvm::GlobalValue::LinkageTypes::ExternalLinkage,
            name,
            *GlobalModule
        );

        for (Block & block : blocks)
        {
            block.initialize(function);
        }
    }
}