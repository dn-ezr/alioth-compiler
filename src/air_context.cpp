#ifndef __air_context_cpp__
#define __air_context_cpp__

#include "air_context.hpp"
#include "diagnostic.hpp"
#include "semantic.hpp"
#include "value.hpp"
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

        /** 初始化reference,unknown等内置数据结构 */
}

AirContext::~AirContext() {
    if( targetMachine ) delete targetMachine;
    targetMachine = nullptr;
}

bool AirContext::operator()( $module semantic ) {
    return translateModule(semantic);
}

bool AirContext::operator()( $module semantic, ostream& os, bool ir ) {
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

    auto ros = llvm::raw_os_ostream(os);
    if( ir ) {
        module->print(ros,nullptr);
    } else {
        llvm::legacy::PassManager pass;
        auto rpos = llvm::buffer_ostream(ros);
        targetMachine->addPassesToEmitFile(pass, rpos, nullptr, llvm::TargetMachine::CGFT_ObjectFile );
        pass.run(*module);
    }

    return true;
}

bool AirContext::translateModule( $module semantics ) {
    module = llvm::make_unique<llvm::Module>((string)semantics->sig->name, *this);

    bool success = true;
    for( auto def : semantics->trans->defs )
        success = translateDefinition(def) and success;
    if( success ) for( auto impl : semantics->impls )
        success = translateImplementation(impl) and success;
    
    if( semantics->entry ) {
        success = generateStartFunction(semantics->entry) and success;
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

bool AirContext::translateClassDefinition( $classdef def ) {
    bool success = true;

    /** 产生布局类型 */
    if( def->targf.size() and def->targs.size() == 0 ) return true;
    auto table = SemanticContext::GetInheritTable(def) << def;
    vector<llvm::Type*> layout_meta;
    vector<llvm::Type*> layout_inst;
    for( auto layout : table )
        for( auto def : layout->defs ) if( auto attr = ($attrdef)def; attr) {
            if( attr->meta ) layout_meta.push_back($(attr->proto));
            else layout_inst.push_back($(attr->proto));
        }

    /** 产生类型 */
    auto entity = $t("entity."+SemanticContext::GetBinarySymbol(($node)def));
    auto instance = $t("struct."+SemanticContext::GetBinarySymbol(($node)def));

    /** 填充结构 */
    entity->setBody(layout_meta);
    instance->setBody(layout_inst);

    /** 迭代内部定义 */
    for( auto sub : def->defs )
        success = translateDefinition(sub) and success;
    
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

bool AirContext::generateStartFunction( $metdef met ) {
    using namespace llvm;
    bool success = true;

    auto start = Function::Create(
        FunctionType::get(Type::getInt32Ty(*this),{Type::getInt32Ty(*this),Type::getInt8PtrTy(*this)->getPointerTo()},false),
        GlobalValue::ExternalLinkage,
        "start",
        module.get()
    );
    auto fp = module->getFunction(SemanticContext::GetBinarySymbol(($node)met));
    if( !fp ) {
        diagnostics[met->getDocUri()]("116", met->name);
        return false;
    }

    auto ebb = BasicBlock::Create(*this,"",start);
    auto builder = IRBuilder<>(ebb);
    auto ret = builder.CreateCall(fp,{
        start->arg_begin(),
        start->arg_begin()+1
    });

    builder.CreateRet( ret );
    return success;
}

llvm::Type* AirContext::$( $eprototype proto ) {
    SemanticContext::$(proto);

    if( proto->dtype->is_type(UnknownType) ) return $(proto->dtype);
    if( proto->etype == eprototype::ref or proto->etype == eprototype::rel ) return $t("reference");
    else return  $(proto->dtype);
}

llvm::Type* AirContext::$( $typeexpr type ) {
    SemanticContext::$(type);
    auto builder = llvm::IRBuilder<>(*this);

    switch( type->id ) {
        case UnknownType: {
            return $t("unknown");
        } break;
        case StructType: case EntityType: {
            return $t(SemanticContext::GetBinarySymbol(($node)type));
        } break;
        case CallableType: {
            auto call = ($callable_type)type->sub;
            vector<llvm::Type*> argts;
            for( auto arg : call->arg_protos ) argts.push_back($(arg));
            return llvm::FunctionType::get(
                $(call->ret_proto),
                argts,
                (bool)call->va_arg
            );
        } break;
        case ConstraintedPointerType: case UnconstraintedPointerType: {
            return $(($typeexpr)type->sub)->getPointerTo();
        } break;
        case NullPointerType: {
            return builder.getVoidTy()->getPointerTo();
        } break;
        case VoidType: {
            return builder.getVoidTy();
        } break;
        case BooleanType: {
            return builder.getInt1Ty();
        } break;
        case Int8Type: case Uint8Type: {
            return builder.getInt8Ty();
        } break;
        case Int16Type: case Uint16Type: {
            return builder.getInt16Ty();
        } break;
        case Int32Type: case Uint32Type: {
            return builder.getInt32Ty();
        } break;
        case Int64Type: case Uint64Type: {
            return builder.getInt64Ty();
        } break;
        case Float32Type: {
            return builder.getFloatTy();
        } break;
        case Float64Type: {
            return builder.getDoubleTy();
        } break;
        case EnumType: {
            return builder.getInt32Ty();
        } break;
    }
    return nullptr;
}

llvm::StructType* AirContext::$t( const string& symbol ) {
    auto& ty = named_types[symbol];
    if( !ty ) ty = llvm::StructType::create(*this, symbol);
    return ty;
}

}
#endif