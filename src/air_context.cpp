#ifndef __air_context_cpp__
#define __air_context_cpp__

#include "air_context.hpp"
#include "diagnostic.hpp"
#include "semantic.hpp"
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/Host.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

namespace alioth {

AirContext::AirContext( string _arch, string _plat, Diagnostics& diag ):
    diagnostics(diag),arch(_arch),platform(_plat),targetMachine(nullptr) {
        using namespace llvm;
        using namespace llvm::sys;
        TargetOptions targetOptions;
        std::string Error;
        auto CPU = "generic";
        auto Features = "";

        /** [TODO]: 支持更多架构 */
        targetTriple = getDefaultTargetTriple();

        LLVMInitializeX86TargetInfo();
        LLVMInitializeX86Target();
        LLVMInitializeX86TargetMC();
        //LLVMInitializeX86AsmParser();
        LLVMInitializeX86AsmPrinter();

        auto target = TargetRegistry::lookupTarget(targetTriple, Error);
        if( !target ) {
            diagnostics["llvm"]("81", Error );
            throw runtime_error("llvm error");
        }
        auto RM = Optional<Reloc::Model>();
        targetMachine = target->createTargetMachine( targetTriple, CPU, Features, targetOptions, RM);
}

AirContext::~AirContext() {
    if( targetMachine ) delete targetMachine;
    targetMachine = nullptr;
}

bool AirContext::operator()( $module semantic, ostream& os ) {
    auto success = translateModule(semantic);

    string source_names;
    auto cctx = semantic->getCompilerContext();
    auto space = cctx.getSpaceEngine();
    for( auto [doc,rec] : semantic->sig->docs )
        source_names += (string)space.getUri(doc) + "; ";
    module->setSourceFileName( source_names );

    string error;
    auto errrso = llvm::raw_string_ostream(error);
    module->setTargetTriple(targetTriple);
    module->setDataLayout(targetMachine->createDataLayout());
    for( auto& fun : module->getFunctionList() )
        if( error.clear(); llvm::verifyFunction(fun, &errrso) )
            diagnostics[semantic->sig->name]("81", error), success = false;
    if( error.clear(); llvm::verifyModule(*module, &errrso) )
        diagnostics[semantic->sig->name]("81", error), success = false;
    if( !success ) return false;

    llvm::legacy::PassManager pass;
    auto ros = llvm::raw_os_ostream(os);
    auto rpos = llvm::buffer_ostream(ros);
    targetMachine->addPassesToEmitFile(pass, rpos, nullptr, llvm::TargetMachine::CGFT_ObjectFile );
    pass.run(*module);

    return true;
}

bool AirContext::translateModule( $module semantic ) {
    module = llvm::make_unique<llvm::Module>((string)semantic->sig->name, *this);

    bool success = true;
    for( auto def : semantic->trans->defs )
        success = translateDefinition(def) and success;
    if( success ) for( auto impl : semantic->impls )
        success = translateImplementation(impl) and success;
    
    if( semantic->sig->entry ) {
        success = generateStartFunction() and success;
    }

    return success;
}

bool AirContext::translateDefinition( $definition def ) {
    if( !def ) return false;
    else if( auto d = ($classdef)def; d ) return translateClassDefinition(d);
    else if( auto d = ($enumdef)def; d ) return translateEnumDefinition(d);
    else if( auto d = ($metdef)def; d ) return translateMethodDefinition(d);
    else if( auto d = ($opdef)def; d ) return translateOperatorDefinition(d);
    else return true;
}

bool AirContext::translateImplementation( $implementation impl ) {
    if( !impl ) return false;
    else if( auto i = ($metimpl)impl; i ) return translateMethodImplementation(i);
    else if( auto i = ($opimpl)impl; i ) return translateOperatorImplementation(i);
    else return false;
}

bool AirContext::translateClassDefinition( $classdef ) {
    bool success = true;
    
    return success;
}

bool AirContext::translateEnumDefinition( $enumdef ) {
    bool success = true;
    
    return success;
}

bool AirContext::translateMethodDefinition( $metdef ) {
    bool success = true;
    
    return success;
}

bool AirContext::translateOperatorDefinition( $opdef ) {
    bool success = true;
    
    return success;
}


bool AirContext::translateMethodImplementation( $metimpl ) {
    bool success = true;
    
    return success;
}

bool AirContext::translateOperatorImplementation( $opimpl ) {
    bool success = true;
    
    return success;
}

bool AirContext::generateStartFunction() {
    bool success = true;
    
    return success;
}


}
#endif