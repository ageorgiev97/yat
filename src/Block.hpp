#pragma once

#include "Llvm.hpp"

#include "Scope.hpp"
#include "Statements.hpp"

namespace yat
{
    struct Block
    {
        llvm::BasicBlock * block = nullptr;
        llvm::Function * function = nullptr;

        std::vector<Statement> statements;

        void initialize(llvm::Function * f);

        void compile(Scope & scope, llvm::BasicBlock * next);
    };
}