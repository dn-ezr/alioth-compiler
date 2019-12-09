#ifndef __test_llvmStructType_cpp__
#define __test_llvmStructType_cpp__

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

    auto t1 = StructType::create(context, "struct");
    cout << "t1:" << t1 << " --- " << (string)t1->getName() << endl;

    auto t2 = StructType::create(context, "struct");
    cout << "t2:" << t2 << " --- " << (string)t2->getName() << endl;

    auto t3 = StructType::create(context);
    t3->setName("struct");
    cout << "t3:" << t3 << " --- " << (string)t3->getName() << endl;

    return 0;
}

#endif