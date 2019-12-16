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
        initInlineStructures();
}

AirContext::~AirContext() {
    if( targetMachine ) delete targetMachine;
    targetMachine = nullptr;
}

void AirContext::initInlineStructures() {
    using namespace llvm;

    alioth = std::make_shared<Module>("alioth", *this);

    auto builder = IRBuilder<>(*this);

    named_types["reference"] = StructType::create(
        "reference",
        builder.getInt8Ty()->getPointerTo(),    // 指向代理体的指针
        builder.getInt32Ty()                    // 偏移量
    );

    named_types["agent"] = StructType::create(
        "agent",
        builder.getInt8Ty()->getPointerTo(),    // 对象实际指针
        builder.getInt32Ty()                    // 引用计数
    );

    named_types["unknown"] = StructType::create(
        "unknown",
        builder.getInt8Ty()->getPointerTo(),    // 对于简单数据类型是值本身，对于复合数据类型是指针
        builder.getInt8Ty()->getPointerTo(),    // 类实体指针，用作RTTI时的TypeId
        builder.getInt32Ty()                    // 变量类型附加信息
    );

    createGlobalStr("void");
    createGlobalStr("boolean");
    createGlobalStr("uint8");
    createGlobalStr("uint16");
    createGlobalStr("uint32");
    createGlobalStr("uint64");
    createGlobalStr("int8");
    createGlobalStr("int16");
    createGlobalStr("int32");
    createGlobalStr("int64");
    createGlobalStr("float32");
    createGlobalStr("float64");
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
    if( llvm::verifyModule(*module, &errrso) )
        errrso.flush(), diagnostics[semantic->sig->name]("81", error), success = false;
    if( !success and !ir ) return false;

    success = generateOutput(module, os, ir) and success;
    return success;
}

bool AirContext::operator()( ostream& os, bool ir ) {
    return generateOutput(alioth, os, ir);
}

bool AirContext::translateModule( $module semantics ) {
    module = std::make_shared<llvm::Module>((string)semantics->sig->name, *this);

    bool success = translateClassDefinition(semantics->trans);
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
    if( def->targf.size() and def->targs.size() == 0 ) {
        for( auto usage : def->usages )
            success = translateClassDefinition(usage) and success;
        return success;
    }
    auto table = SemanticContext::GetInheritTable(def) << def;
    vector<llvm::Type*> layout_meta;
    vector<llvm::Type*> layout_inst;
    for( auto layout : table )
        for( auto def : layout->defs ) if( auto attr = ($attrdef)def; attr) {
            if( attr->meta ) layout_meta.push_back($t(attr));
            else layout_inst.push_back($t(attr));
        }

    /** 产生类型 */
    auto entity_ty = (llvm::StructType*)$et(def);
    auto instance_ty = (llvm::StructType*)$t(def);

    /** 填充结构 */
    entity_ty->setBody(layout_meta);
    instance_ty->setBody(layout_inst);

    auto entity = $e(def);
    entity->setInitializer(llvm::ConstantAggregateZero::get(entity_ty));

    /** 迭代内部定义 */
    for( auto sub : def->defs )
        success = translateDefinition(sub) and success;
    
    return success;
}

bool AirContext::translateEnumDefinition( $enumdef ) {
    bool success = true;
    
    return success;
}

bool AirContext::translateMethodDefinition( $metdef def ) {
    using namespace llvm;

    bool success = true;

    module->getOrInsertFunction(SemanticContext::GetBinarySymbol(($node)def),$t(def));
    
    return success;
}

bool AirContext::translateOperatorDefinition( $opdef def ) {
    bool success = true;
    
    return success;
}


bool AirContext::translateMethodImplementation( $metimpl impl ) {
    using namespace llvm;

    bool success = true;

    /** 获取函数 */
    auto def = ($metdef)SemanticContext::GetDefinition(($implementation)impl);
    auto ft = $t(def);
    auto fp = (Function*)module->getOrInsertFunction(SemanticContext::GetBinarySymbol(($node)impl),ft);
    auto bb = BasicBlock::Create(*this, "", fp );
    auto builder = IRBuilder<>(bb);
    auto attrs = $a(def);

    /** 存储参数以获取地址 */
    auto argi = fp->arg_begin();
    if( (attrs&metattr::retcm) and (attrs&metattr::tsarg) ) {
        argi++->setName("return");
        argi++->setName("this");
    } else if( attrs&metattr::retcm ) {
        argi++->setName("return");
    } else if( attrs&metattr::tsarg ) {
        argi++->setName("this");
    }
    for( auto arg : impl->arguments ) {
        argi->setName((string)arg->name);
        if( arg->proto->etype == eprototype::obj and arg->proto->dtype->is_type(StructType) ) {
            element_values[arg] = argi;
        } else {
            auto addr = element_values[arg] = builder.CreateAlloca($t(arg->proto));
            builder.CreateStore(argi,addr);
        }
        argi++;
    }

    // success = translateBlockStatement(impl->body) and success;
    
    return success;
}

bool AirContext::translateOperatorImplementation( $opimpl impl ) {
    using namespace llvm;
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

bool AirContext::generateOutput( shared_ptr<llvm::Module> mod, ostream& os, bool ir ) {
    bool success = true;
    mod->setTargetTriple(targetTriple);
    mod->setDataLayout(targetMachine->createDataLayout());

    auto ros = llvm::raw_os_ostream(os);
    if( ir ) {
        mod->print(ros,nullptr);
    } else {
        llvm::legacy::PassManager pass;
        auto rpos = llvm::buffer_ostream(ros);
        targetMachine->addPassesToEmitFile(pass, rpos, nullptr, llvm::TargetMachine::CGFT_ObjectFile );
        pass.run(*mod);
    }

    return true;
}

llvm::StructType* AirContext::$t( $classdef def ) {
    return $t("struct."+SemanticContext::GetBinarySymbol(($node)def));
}

llvm::StructType* AirContext::$et( $classdef def ) {
    return $t("entity_struct."+SemanticContext::GetBinarySymbol(($node)def));
}

llvm::Type* AirContext::$t( $attrdef attr ) {
    auto type = $t(attr->proto);
    if( type and attr->arr.size() ) {
        auto c = 1;
        for( auto arr : attr->arr ) c *= arr;
        type = llvm::ArrayType::get(type, c);
    }
    return type;
}

llvm::FunctionType* AirContext::$t( $metdef def ) {
    using namespace llvm;

    vector<Type*> args;
    auto attrs = $a(def);
    Type* ret = nullptr;
    for( auto arg : def->arguments ) 
        args.push_back($t(arg->proto));
    if( attrs&metattr::tsarg )
        args.insert(args.begin(), $t(SemanticContext::GetThisClassDef(($node)def))->getPointerTo());
    if( attrs&metattr::retri ) {
        args.push_back($t("unknown")->getPointerTo());
        ret = Type::getInt32Ty(*this);
    }else if( attrs&metattr::retst ) {
        args.push_back($t(def->ret_proto)->getPointerTo());
        ret = Type::getInt32Ty(*this);
    } else if( attrs&metattr::retrf ) {
        args.push_back($t("reference")->getPointerTo());
        ret = Type::getInt32Ty(*this);
    } else {
        ret = $t(def->ret_proto);
    }
    auto ft = FunctionType::get(
        ret,
        args,
        (bool)def->va_arg
    );

    return ft;
}

llvm::Type* AirContext::$t( $eprototype proto ) {
    SemanticContext::$(proto);

    if( proto->dtype->is_type(UnknownType) ) return $t(proto->dtype);
    if( proto->etype == eprototype::ref or proto->etype == eprototype::rel ) return $t("reference");
    else return  $t(proto->dtype);
}

llvm::Type* AirContext::$t( $typeexpr type ) {
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
            for( auto arg : call->arg_protos ) argts.push_back($t(arg));
            return llvm::FunctionType::get(
                $t(call->ret_proto),
                argts,
                (bool)call->va_arg
            );
        } break;
        case ConstraintedPointerType: case UnconstraintedPointerType: {
            return $t(($typeexpr)type->sub)->getPointerTo();
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

metattrs AirContext::$a( $metdef def ) {
    if( method_attrs.count(def) ) return method_attrs[def];

    auto& attrs = method_attrs[def];

    if( def->arguments.size() == 0 ) attrs |= metattr::noarg;
    if( not def->meta ) attrs |= metattr::tsarg;
    if( def->ret_proto->dtype->is_type(UnknownType) ) attrs |= metattr::retri;
    if( def->ret_proto->etype == eprototype::obj and def->ret_proto->dtype->is_type(StructType) ) attrs |= metattr::retst;
    if( def->ret_proto->etype == eprototype::ref ) attrs |= metattr::retrf;
    if( def->va_arg ) attrs |= metattr::vaarg;

    return attrs;
}

llvm::StructType* AirContext::$t( const string& symbol ) {
    auto& ty = named_types[symbol];
    if( !ty ) ty = llvm::StructType::create(*this, symbol);
    return ty;
}

llvm::GlobalVariable* AirContext::$e( $classdef def ) {
    auto entity_ty = $et(def);
    auto entity_nm = "entity."+SemanticContext::GetBinarySymbol(($node)def);
    return (llvm::GlobalVariable*)module->getOrInsertGlobal(entity_nm, entity_ty);
}

llvm::GlobalVariable* AirContext::createGlobalStr( const string& str ) {
    using namespace llvm;

    auto builder = IRBuilder<>(*this);

    vector<Constant*> init;
    for( auto c : str ) init.push_back(builder.getInt8(c));
    init.push_back(builder.getInt8(0));

    auto ai8 = ArrayType::get(builder.getInt8Ty(), str.length()+1);
    auto arr = ConstantArray::get(ai8, init);

    return new GlobalVariable(*alioth, ai8, true, GlobalValue::ExternalLinkage, arr, str);
}

}
#endif