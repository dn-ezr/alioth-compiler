#ifndef __test_llvmReferenceAcrossModule_cpp__
#define __test_llvmReferenceAcrossModule_cpp__

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <iostream>

int main( int argc, char** argv ) {
    using namespace llvm;
    using namespace std;

    auto os = raw_os_ostream(cout);
    auto context = LLVMContext();
    auto module = llvm::make_unique<Module>("alioth", context);
    auto builder = IRBuilder<>(context);

    auto gv1 = new GlobalVariable(
        *module,
        builder.getInt8Ty(), 
        true, GlobalValue::ExternalLinkage,
        builder.getInt8(12),
        "gv1");
    cout << "--------------------------------" << endl;
    module->print(os, nullptr);
    cout << "--------------------------------" << endl;

    auto mod = llvm::make_unique<Module>("mod", context);

    auto gv2 = new GlobalVariable(
        *mod,
        builder.getInt8Ty(), 
        true, GlobalValue::ExternalLinkage,
        builder.getInt8(24),
        "gv2");
    
    auto ft = FunctionType::get(builder.getVoidTy(), false);
    auto fp = Function::Create(ft, GlobalValue::ExternalLinkage, "test", *mod);
    auto bb = BasicBlock::Create(context, "", fp);
    builder.SetInsertPoint(bb);

    auto p = builder.CreateAlloca(builder.getInt8Ty(), nullptr);

    auto v = builder.CreateLoad(mod->getOrInsertGlobal("gv1", builder.getInt8Ty()));

    builder.CreateStore(v,p);

    builder.CreateRetVoid();

    verifyModule(*mod, &os);

    mod->print(os, nullptr);

    return 0;
}

#endif