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

exe_scope::exe_scope( $scope sc, type_t ty ):scope(sc),type(ty),parent(sc->getScope()) {

}

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
    createGlobalStr("bool");
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

    bool success = true;

    module->getOrInsertFunction(SemanticContext::GetBinarySymbol(($node)def),$t(def));
    
    return success;
}

bool AirContext::translateOperatorDefinition( $opdef def ) {
    bool success = true;

    module->getOrInsertFunction(SemanticContext::GetBinarySymbol(($node)def),$t(def));
    
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

    auto scope = enter(impl, exe_scope::Origin);

    /** 存储参数以获取地址 */
    auto argi = fp->arg_begin();
    if( attrs&metattr::retcm ) {
        $element rt = new element;
        rt->name = token("return");
        element_values[rt] = argi;
        scope->elements[rt->name] = rt;
        argi++->setName("return");
    }
    if( attrs&metattr::tsarg ) {
        $element ts = new element;
        ts->name = token("this");
        ts->proto = eprototype::make(
            impl, token("this"), typeexpr::make(
                impl, token("this"), 
                (def->cons?ConstraintedPointerType:UnconstraintedPointerType),
                (anything)typeexpr::make(
                    impl, token("this"), StructType, def->getScope()
                )
            ), eprototype::ptr, token("const")
        );
        element_values[ts] = argi;
        scope->elements[ts->name] = ts;
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
        scope->elements[arg->name] = arg;
        argi++;
    }

    success = translateBlockStatement(builder, impl->body) and success;
    
    return success;
}

bool AirContext::translateOperatorImplementation( $opimpl impl ) {
    using namespace llvm;
    bool success = true;
    not_ready_yet;
    return success;
}

bool AirContext::translateStatement( llvm::IRBuilder<>& builder, $statement stmt ) {
    if( auto s = ($blockstmt)stmt; s ) {
        return translateBlockStatement(builder, s);
    } else if( auto s = ($element)stmt; s ) {
        return translateElementStatement(builder, s);
    } else if( auto s = ($branchstmt)stmt; s ) {
        return translateBranchStatement(builder, s);
    } else if( auto s = ($switchstmt)stmt; s ) {
        return translateSwitchStatement(builder, s);
    } else if( auto s = ($assumestmt)stmt; s ) {
        return translateAssumeStatement(builder, s);
    } else if( auto s = ($loopstmt)stmt; s ) {
        return translateLoopStatement(builder, s);
    } else if( auto s = ($fctrlstmt)stmt; s ) {
        return translateFlowControlStatement(builder, s);
    } else if( auto s = ($dostmt)stmt; s ) {
        return translateDoStatement(builder, s);
    } else if( auto s = ($exprstmt)stmt; s ) {
        return translateExpressionStatement(builder, s );
    } else {
        return false;
    }
}

bool AirContext::translateBlockStatement( llvm::IRBuilder<>& builder, $blockstmt stmt ) {
    bool success = true;
    for( auto s : *stmt ) 
        success = translateStatement(builder,s) and success;
    return success;
}

bool AirContext::translateElementStatement( llvm::IRBuilder<>& builder, $element stmt ) {
    not_ready_yet;
    return false;
}

bool AirContext::translateBranchStatement( llvm::IRBuilder<>& builder, $branchstmt stmt ) {
    not_ready_yet;
    return false;
}

bool AirContext::translateSwitchStatement( llvm::IRBuilder<>& builder, $switchstmt stmt ) {
    not_ready_yet;
    return false;
}

bool AirContext::translateAssumeStatement( llvm::IRBuilder<>& builder, $assumestmt stmt ) {
    not_ready_yet;
    return false;
}

bool AirContext::translateLoopStatement( llvm::IRBuilder<>& builder, $loopstmt stmt ) {
    not_ready_yet;
    return false;
}

bool AirContext::translateFlowControlStatement( llvm::IRBuilder<>& builder, $fctrlstmt stmt ) {
    bool success = true;
    if( stmt->action.is(VT::BREAK) ) {

    } else if( stmt->action.is(VT::O::FORCE) ) {

    } else if( stmt->action.is(VT::CONTINUE) ) {

    } else if( stmt->action.is(VT::RETURN) ) {
        if( stmt->expr ) {
            auto ex = translateExpressionStatement(builder, stmt->expr);
            if( !ex ) return false;
            builder.CreateRet(ex->value);
        } else {
            builder.CreateRetVoid();
        }
    }
    return success;
}

bool AirContext::translateDoStatement( llvm::IRBuilder<>& builder, $dostmt stmt ) {
    not_ready_yet;
    return false;
}

$value AirContext::translateExpressionStatement( llvm::IRBuilder<>& builder, $exprstmt stmt ) {
    if( auto expr = ($monoexpr)stmt; expr ) {
        return translateMonoExpression( builder, expr );
    } else if( auto expr = ($binexpr)stmt; expr ) {
        return translateBinaryExpression( builder, expr );
    } else if( auto expr = ($callexpr)stmt; expr ) {
        return translateCallExpression( builder, expr );
    } else if( auto expr = ($tconvexpr)stmt; expr ) {
        return translateTypeConvertExpression( builder, expr );
    } else if( auto expr = ($aspectexpr)stmt; expr ) {
        return translateAspectExpression( builder, expr );
    } else if( auto expr = ($mbrexpr)stmt; expr ) {
        return translateMemberExpression( builder, expr );
    } else if( auto expr = ($nameexpr)stmt; expr ) {
        return translateNameExpression( builder, expr );
    } else if( auto expr = ($constant)stmt; expr ) {
        return translateConstantExpression( builder, expr );
    } else if( auto expr = ($lctorexpr)stmt; expr ) {
        return translateListConstructingExpression( builder, expr );
    } else if( auto expr = ($sctorexpr)stmt; expr ) {
        return translateStructuralConstructingExpression( builder, expr );
    } else if( auto expr = ($tctorexpr)stmt; expr ) {
        return translateTupleConstructingExpression( builder, expr );
    } else if( auto expr = ($lambdaexpr)stmt; expr ) {
        return translateLambdaExpression( builder, expr );
    } else if( auto expr = ($newexpr)stmt; expr ) {
        return translateNewExpression( builder, expr );
    } else if( auto expr = ($delexpr)stmt; expr ) {
        return translateDeleteExpression( builder, expr );
    } else if( auto expr = ($doexpr)stmt; expr ) {
        return translateDoExpression( builder, expr );
    } else {
        internal_error;
        return nullptr;
    }
}

$value AirContext::translateMonoExpression( llvm::IRBuilder<>& builder, $monoexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateBinaryExpression( llvm::IRBuilder<>& builder, $binexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateCallExpression( llvm::IRBuilder<>& builder, $callexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateTypeConvertExpression( llvm::IRBuilder<>& builder, $tconvexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateAspectExpression( llvm::IRBuilder<>& builder, $aspectexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateMemberExpression( llvm::IRBuilder<>& builder, $mbrexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateNameExpression( llvm::IRBuilder<>& builder, $nameexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateConstantExpression( llvm::IRBuilder<>& builder, $constant expr ) {
    switch( expr->value.id ) {
        case VT::L::CHAR : {
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, NamedType,
                (anything)nameexpr::make(expr->getScope(),expr->phrase,token("char")) );
            not_ready_yet;
            return nullptr;
        } break;
        case VT::L::STRING : {
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, NamedType,
                (anything)nameexpr::make(expr->getScope(),expr->phrase,token("string")) );
            not_ready_yet;
            return nullptr;
        } break;
        case VT::L::FALSE : {
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, BooleanType);
            $value value = new value_t;
            value->addr = none;
            value->value = builder.getFalse();
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::TRUE : {
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, BooleanType);
            $value value = new value_t;
            value->addr = none;
            value->value = builder.getTrue();
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::FLOAT : {
            $value value = new value_t;
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, Float64Type);
            value->addr = none;
            value->value = llvm::ConstantFP::get(*this, llvm::APFloat(stod(expr->value.tx)));
            value->proto = eprototype::make(expr->getScope(), expr->phrase, dtype);
            return value;
        } break;
        case VT::L::NULL : {
            auto dtype = typeexpr::make(expr->getScope(),expr->phrase, NullPointerType);
            $value value = new value_t;
            value->addr = none;
            value->value = builder.getInt64(0);
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::I::B : {
            const auto bits = expr->value.tx.size()-2;
            const auto bytes = (bits+7)/8;
            $typeexpr dtype;
            $value value = new value_t;
            value->addr = none;
            auto v = stoull(string(expr->value.tx.begin()+2, expr->value.tx.end()),nullptr, 2);
            if( bytes <= 1 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint8Type );
                value->value = builder.getInt8(v);
            } else if( bytes <= 2 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint16Type );
                value->value = builder.getInt16(v);
            } else if( bytes <= 4 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint32Type );
                value->value = builder.getInt32(v);
            } else if( bytes <= 8 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint64Type );
                value->value = builder.getInt64(v);
            } else return nullptr;
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::I::H : {
            const auto bits = expr->value.tx.size()-2;
            const auto bytes = (bits+1)/2;
            $typeexpr dtype;
            $value value = new value_t;
            value->addr = none;
            auto v = stoull(string(expr->value.tx.begin()+2, expr->value.tx.end()),nullptr, 16);
            if( bytes <= 1 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint8Type );
                value->value = builder.getInt8(v);
            } else if( bytes <= 2 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint16Type );
                value->value = builder.getInt16(v);
            } else if( bytes <= 4 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint32Type );
                value->value = builder.getInt32(v);
            } else if( bytes <= 8 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint64Type );
                value->value = builder.getInt64(v);
            } else return nullptr;
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::I::O : {
            const auto bits = expr->value.tx.size()-2;
            const auto bytes = (bits+2)/3;
            $typeexpr dtype;
            $value value = new value_t;
            value->addr = none;
            auto v = stoull(string(expr->value.tx.begin()+2, expr->value.tx.end()),nullptr, 8);
            if( bytes <= 1 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint8Type );
                value->value = builder.getInt8(v);
            } else if( bytes <= 2 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint16Type );
                value->value = builder.getInt16(v);
            } else if( bytes <= 4 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint32Type );
                value->value = builder.getInt32(v);
            } else if( bytes <= 8 ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint64Type );
                value->value = builder.getInt64(v);
            } else return nullptr;
            value->proto = eprototype::make(expr->getScope(),expr->phrase,dtype);
            return value;
        } break;
        case VT::L::I::N : {
            const auto n = stoull(expr->value.tx);
            $typeexpr dtype;
            $value value = new value_t;
            value->addr = none;
            if( n <= INT32_MAX ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Int32Type );
                value->value = builder.getInt32(n);
            } else if( n <= UINT32_MAX ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint32Type );
                value->value = builder.getInt32(n);
            } else if( n <= INT64_MAX ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Int64Type );
                value->value = builder.getInt64(n);
            } else if( n <= UINT64_MAX ) {
                dtype = typeexpr::make(expr->getScope(),expr->phrase, Uint64Type );
                value->value = builder.getInt64(n);
            } else return nullptr;
            value->proto = eprototype::make(expr->getScope(), expr->phrase, dtype);
            return value;
        } break;
        case VT::L::THIS : {
            auto impl = $impl(($statement)expr);
            if( !impl ) return internal_error, nullptr;
            if( auto ts = $el("this", impl); ts ) {
                $value value = new value_t;
                value->addr = none;
                value->proto = ts->proto;
                value->value = element_values[ts];
                return value;
            } else {
                diagnostics("121", expr->phrase);
                return nullptr;
            }
        } break;
        case VT::CLASS : {
            $value value = new value_t;
            value->addr = none;
            value->proto = eprototype::make(
                expr->getScope(), token("this class"), typeexpr::make(
                    expr->getScope(), expr->phrase, ThisClassType));
            value->value = $e(SemanticContext::GetThisClassDef(($node)expr));
            return value;
        } break;
        case VT::L::LABEL: {
            $value value = new value_t;
            value->addr = none;
            auto def = ($enumdef)expr->getScope();
            if( !def ) return internal_error, nullptr;
            value->proto = eprototype::make(
                expr->getScope(), expr->phrase, typeexpr::make(
                    expr->getScope(), expr->phrase, EnumType, expr->getScope() ));
            value->value = builder.getInt32(def->items.index(expr));
            return value;
        } break;
        default: return internal_error, nullptr;
    }
    return nullptr;
}

$value AirContext::translateListConstructingExpression( llvm::IRBuilder<>& builder, $lctorexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateStructuralConstructingExpression( llvm::IRBuilder<>& builder, $sctorexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateTupleConstructingExpression( llvm::IRBuilder<>& builder, $tctorexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateLambdaExpression( llvm::IRBuilder<>& builder, $lambdaexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateNewExpression( llvm::IRBuilder<>& builder, $newexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateDeleteExpression( llvm::IRBuilder<>& builder, $delexpr expr ) {
    not_ready_yet;
    return nullptr;
}

$value AirContext::translateDoExpression( llvm::IRBuilder<>& builder, $doexpr expr ) {
    not_ready_yet;
    return nullptr;
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

$exe_scope AirContext::enter($scope scope, exe_scope::type_t type) {
    if( scopes.count(scope) ) return scopes[scope];
    return scopes[scope] = new exe_scope(scope,type);
}

void AirContext::leave($scope scope) {
    scopes.erase(scope);
}

$exe_scope AirContext::$sc($scope scope) {
    while( scope != nullptr ) {
        if( scopes.count(scope) ) return scopes[scope];
        scope = scope->getScope();
    }
    return nullptr;
}

$element AirContext::$el( const string& name, $scope scope ) {
    for( auto esc = $sc(scope); esc; esc = $sc(esc->parent) ) {
        if( esc->elements.count(name) )
            return esc->elements[name];
    }
    return nullptr;
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

llvm::FunctionType* AirContext::$t( $opdef def ) {
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

    auto& attrs = method_attrs[def] = 0;

    if( def->arguments.size() == 0 ) attrs |= metattr::noarg;
    if( not def->meta ) attrs |= metattr::tsarg;
    if( def->ret_proto->dtype->is_type(UnknownType) ) attrs |= metattr::retri;
    if( def->ret_proto->etype == eprototype::obj and def->ret_proto->dtype->is_type(StructType) ) attrs |= metattr::retst;
    if( def->ret_proto->etype == eprototype::ref ) attrs |= metattr::retrf;
    if( def->va_arg ) attrs |= metattr::vaarg;

    return attrs;
}

metattrs AirContext::$a( $opdef def ) {
    if( op_attrs.count(def) ) return op_attrs[def];

    auto& attrs = op_attrs[def] = 0;

    attrs |= metattr::tsarg;
    if( def->arguments.size() == 0 ) attrs |= metattr::noarg;
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

$implementation AirContext::$impl( $statement stmt ) {
    $node n = ($node)stmt;
    while( n and !n->is(node::IMPLEMENTATION) )
        n = n->getScope();
    return ($implementation)n;
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