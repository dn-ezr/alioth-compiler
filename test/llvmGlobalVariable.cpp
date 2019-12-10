#ifndef __test_llvmGlobalVariable_cpp__
#define __test_llvmGlobalVariable_cpp__

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

    auto context = LLVMContext();
    auto module = llvm::make_unique<Module>("alioth", context);
    auto builder = IRBuilder<>(context);

    auto gv1 = new GlobalVariable(
        *module,
        ArrayType::get(builder.getInt8Ty(), 8), 
        true, GlobalValue::ExternalLinkage,
        ConstantArray::get(
            ArrayType::get(builder.getInt8Ty(), 8),
            {
                builder.getInt8('f'),
                builder.getInt8('l'),
                builder.getInt8('o'),
                builder.getInt8('a'),
                builder.getInt8('t'),
                builder.getInt8('6'),
                builder.getInt8('4'),
                builder.getInt8(0)
            }
        ),
        "float64");

    cout << "gv1: " << gv1 << " --- " << (string)gv1->getName() << endl;
    cout << "float64: " << module->getNamedGlobal("float64") << endl;
    
    auto os = raw_os_ostream(cout);
    module->print(os, nullptr);

    return 0;
}

#endif