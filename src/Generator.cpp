#include "Generator.hpp"

#include <iostream>

namespace yat
{
    std::vector<Scope> scopes;

    llvm::LLVMContext GlobalContext;
    std::unique_ptr<llvm::Module> GlobalModule = std::make_unique<llvm::Module>("main", GlobalContext);

    std::unordered_map<std::string, Function> functions;

    void GenerateCode(Statement const& statement)
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::Function * main_function = llvm::Function::Create(
            llvm::FunctionType::get(
                llvm::Type::getInt32Ty(GlobalContext),
                false
            ),
            llvm::GlobalValue::LinkageTypes::ExternalLinkage,
            "main",
            *GlobalModule
        );

        llvm::BasicBlock * block = llvm::BasicBlock::Create(GlobalContext, "entry", main_function, 0);
        Scope scope;

        statement.compile(scope, main_function, block, nullptr);

        //llvm::ReturnInst::Create(GlobalContext, expression.compile(scopes.back()), block);
        //llvm::ReturnInst::Create(GlobalContext, llvm::ConstantInt::get(GlobalContext, llvm::APInt(32, 0, true)), block);

        llvm::legacy::PassManager manager;
        manager.add(llvm::createPrintModulePass(llvm::outs()));
        manager.run(*GlobalModule);

        llvm::EngineBuilder eb(std::move(GlobalModule));
        eb.setEngineKind(llvm::EngineKind::JIT);

        llvm::ExecutionEngine * ee = eb.create();
        ee->finalizeObject();

        std::vector<llvm::GenericValue> noargs;
        int exit_code = ee->runFunctionAsMain(main_function, {}, nullptr);

        std::cout << "Exit code: " << exit_code << std::endl;
    }
}