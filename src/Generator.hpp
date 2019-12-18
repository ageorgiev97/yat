#pragma once

#include <memory>
#include <unordered_map>

#include "Llvm.hpp"

#include "Expressions.hpp"
#include "Statements.hpp"
#include "Scope.hpp"
#include "Function.hpp"

namespace yat
{
    extern llvm::LLVMContext GlobalContext;
    extern std::unique_ptr<llvm::Module> GlobalModule;

    extern std::unordered_map<std::string, Function> functions;

    void GenerateCode(Statement const& expression);
}