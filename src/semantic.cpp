#ifndef __semantic_cpp__
#define __semantic_cpp__

#include "semantic.hpp"

namespace alioth {

module::module( SemanticContext& context ):sctx(context) {

}

bool module::is( type t )const {
    return t == MODULE;
}

$module module::getModule() {
    return this;
}

CompilerContext& module::getCompilerContext() {
    return sig->getCompilerContext();
}

SemanticContext::SemanticContext( CompilerContext& _cctx, Diagnostics& diag ):
    cctx(_cctx),diagnostics(diag) {

}

bool SemanticContext::associateModule( $signature sig ) {
    if( !sig ) return false;
    if( auto it = forest.find(sig); it != forest.end() and it->second ) return true;
    auto& mod = forest[sig] = new module(*this);
    mod->sig = sig;
    for( auto [_,rec] : sig->docs ) {
        auto fg = rec.fg;
        mod->defs += fg->defs;
        mod->impls += fg->impls;
        fg->setScope(mod);
    }

    $definition* trans = nullptr;
    bool success = true;
    for( auto& def : mod->defs ) {
        if( auto cdef = ($classdef)def; cdef and (string)cdef->name == (string)mod->sig->name ) {
            if( trans )
                success = (diagnostics
                    [cdef->getDocUri()]("77", cdef->name)
                    [-1]((*trans)->getDocUri(),"45",(*trans)->name), false);
            if( cdef->targf.size() )
                success = (diagnostics[cdef->getDocUri()]("74", cdef->name), false);
            if( cdef->abstract )
                success = (diagnostics[cdef->getDocUri()]("75", cdef->name), false);
            if( cdef->supers.size() )
                success = (diagnostics[cdef->getDocUri()]("76", cdef->name), false);
            mod->defs += cdef->contents;
            trans = &def;
        }
    }
    if( trans ) mod->defs.remover(*trans);

    if( !success ) mod = nullptr;
    return success;
}

bool SemanticContext::associateModules( signatures all ) {
    bool success = true;
    for( auto sig : all )
        success = associateModule(sig) and success;
    return success;
}

bool SemanticContext::releaseModule( $signature sig ) {
    return forest.erase(sig);
}

void SemanticContext::releaseModules( signatures sigs ) {
    for( auto sig : sigs ) releaseModule(sig);
}

$module SemanticContext::getModule( $signature sig ) {
    auto it = forest.find(sig);
    if( it == forest.end() ) return nullptr;
    return it->second;
}

$module SemanticContext::getModule( $depdesc dep ) {
    auto it = dep_cache.find(dep);
    if( it != dep_cache.end() ) return it->second;

    auto sig = ($signature)dep->getScope();
    if( !sig ) return internal_error, nullptr;

    modules search;
    for( auto [sig,mod] : forest ) if( sig->name == dep->name ) search << mod;

    $module result;
    auto from = (string)dep->from;
    if( from == "." ) {
        for( auto mod : search ) if( mod->sig->space == sig->space ) {
            result = mod;
            break;
        }
    } else if( from == "alioth" ) {
        for( auto mod : search ) if( mod->sig->space.flags == ROOT ) {
            result = mod;
            break;
        }
    } else if( from.size() ) {
        srcdesc desc = {flags:APKG,package:from};
        for( auto mod : search ) if( mod->sig->space == desc ) {
            result = mod;
            break;
        }
    } else {
        for( auto mod : search ) if( mod->sig->space == sig->space ) {
            result = mod;
            break;
        }
        if( !result ) for( auto mod : search ) if( mod->sig->space.flags == ROOT ) {
            result = mod;
            break;
        }
    }

    if( !result ) return internal_error, nullptr;
    return result;
}

bool SemanticContext::validateDefinitionSemantics() {
    bool success = true;
    for( auto [sig,mod] : forest )
        success = mod and validateModuleDefinition(mod) and success;
    return success;
}

bool SemanticContext::validateImplementationSemantics() {
    bool success = true;
    for( auto [sig,mod] : forest ) {
        if( !mod ) {
            success = false;
        } else for( auto impl : mod->impls ) {
            if( auto op = ($opimpl)impl; op ) success = validateOperatorImplementation(op) and success;
            else if( auto mt = ($metimpl)impl; mt ) success = validateMethodImplementation(mt) and success;
            else internal_error, success = false;
        }
    }
    return success;
}

bool SemanticContext::validateModuleDefinition( $module mod ) {
    bool success = true;

    /** 检查定义语义以及重复定义 */
    for( auto& def : mod->defs ) {

        if( auto cldef = ($classdef)def; cldef ) success = validateClassDefinition(cldef) and success;
        else if( auto endef = ($enumdef)def; endef ) success =  validateEnumDefinition( endef ) and success;
        else if( auto aldef = ($aliasdef)def; aldef ) success =  validateAliasDefinition( aldef ) and success;
        else if( auto atdef = ($attrdef)def; atdef ) success =  validateAttributeDefinition( atdef ) and success;
        else if( auto medef = ($metdef)def; medef ) success =  validateMethodDefinition( medef ) and success;
        else if( auto opdef = ($opdef)def; opdef ) success =  validateOperatorDefinition( opdef ) and success;
        else internal_error, success = false;

        /** 检查重复定义 */
        for( auto& prv : mod->defs ) {
            if( &def == &prv ) break;
            if( def->name != prv->name ) continue;
            bool repeat = true;

            string dname = GetBinarySymbol(($node)def);
            string pname = GetBinarySymbol(($node)prv);
            if( def->is(node::METHODDEF) and prv->is(node::METHODDEF) ) repeat = dname == pname;

            if( repeat ) {
                diagnostics[def->getDocUri()]("98", def->name, dname)
                [-1](prv->getDocUri(), "45", prv->name);
                success = false;
            }
        }
    }

    return success;
}

bool SemanticContext::validateClassDefinition(  $classdef cls ) {
    bool success = true;

    /** 忽略模板类,只有模板类用例才会被检查 */
    if( cls->targf.size() and !cls->targs.size() ) return true;

    /** 检查基类是否可达 */
    for( auto super : cls->supers ) {
        auto res = $(super);
        if( res.size() != 1 ) {
            diagnostics[cls->getDocUri()]("82", super->phrase);
            success = false;
            continue;
        }
        auto def = ($classdef)res[0];
        if( !def or !CanBeInstanced(def) ) {
            diagnostics[cls->getDocUri()]("83", super->phrase);
            success = false;
            continue;
        }
    }

    /** 检查定义语义以及重复定义 */
    for( auto& def : cls->contents ) {

        if( auto cldef = ($classdef)def; cldef ) success = validateClassDefinition(cldef) and success;
        else if( auto endef = ($enumdef)def; endef ) success =  validateEnumDefinition( endef ) and success;
        else if( auto aldef = ($aliasdef)def; aldef ) success =  validateAliasDefinition( aldef ) and success;
        else if( auto atdef = ($attrdef)def; atdef ) success =  validateAttributeDefinition( atdef ) and success;
        else if( auto medef = ($metdef)def; medef ) success =  validateMethodDefinition( medef ) and success;
        else if( auto opdef = ($opdef)def; opdef ) success =  validateOperatorDefinition( opdef ) and success;
        else internal_error, success = false;

        /** 检查重复定义 */
        for( auto& prv : cls->contents ) {
            if( &def == &prv ) break;
            if( def->name != prv->name ) continue;
            bool repeat = true;

            string dname = GetBinarySymbol(($node)def);
            string pname = GetBinarySymbol(($node)prv);
            if( def->is(node::METHODDEF) and prv->is(node::METHODDEF) ) repeat = dname == pname;

            if( repeat ) {
                diagnostics[def->getDocUri()]("98", def->name, dname)
                [-1](prv->getDocUri(), "45", prv->name);
                success = false;
            }
        }

        /** 检查谓词 */
        //[TODO]
    }

    /** 检查循环包含 */
    if( success ) {
        function<bool($classdef def, classdefs padding)> check = [this,&check]( $classdef def, classdefs paddings) {
            for( auto padding : paddings ) if( padding == def ) return false;

            /** 检查基类循环 */
            for( auto super : def->supers ) {
                auto sdef = ReachClass(super);
                if( !sdef ) return internal_error, false;
                if( !check( sdef, paddings + classdefs{def} ) ) {
                    if( paddings.size() == 0 ) diagnostics[def->getDocUri()]("91", super->phrase );
                    return false;
                }
            }

            /** 检查成员循环包含 */
            for( auto content : def->contents ) {
                if( auto attr = ($eprototype)content ) {
                    $(attr);
                    if( attr->etype == eprototype::obj and attr->dtype->is_type(StructType) )
                        if( !check( ($classdef)attr->dtype->sub, paddings + classdefs{def}) ) {
                            if( paddings.size() == 0 ) diagnostics[def->getDocUri()]("92", attr->phrase);
                            return false;
                        }
                }
            }
            return true;
        };
        success = check( cls, {} ) and success;
    }

    return success;
}

bool SemanticContext::validateEnumDefinition(  $enumdef def ) {
    bool success = true;

    /**
     * Nothing to be done
     */

    return success;
}

bool SemanticContext::validateAliasDefinition(  $aliasdef def ) {
    bool success = true;

    auto result = $(def->tagret);

    success = result.size();

    return success;
}

bool SemanticContext::validateAttributeDefinition(  $attrdef def ) {
    bool success = true;

    if( !$(def->proto) ) {
        success = false;
    } else if( def->proto->etype == eprototype::obj and def->proto->dtype->is_type(PointerTypeMask) ) {
        success = false;
        diagnostics[def->getDocUri()]("42", def->phrase);
    } else if( def->proto->etype == eprototype::ptr and !def->proto->dtype->is_type(PointerTypeMask) ) {
        success = false;
        diagnostics[def->getDocUri()]("42", def->phrase);
    }

    return success;
}

bool SemanticContext::validateMethodDefinition(  $metdef def ) {
    bool success = true;
    for( auto proto : def->arg_protos ) {
        if( !$(proto) ) success = false;
    }
    if( !$(def->ret_proto) ) success = false;
    return success;
}

bool SemanticContext::validateOperatorDefinition(  $opdef def ) {
    bool success;

    if( def->name.is(
        PVT::ADD,PVT::SUB,PVT::MUL,PVT::DIV,PVT::MOL,
        PVT::BITAND,PVT::BITOR,PVT::BITXOR,PVT::SHL,PVT::SHR,
        PVT::LT,PVT::GT,PVT::LE,PVT::GE,PVT::EQ,PVT::NE,
        PVT::AND,PVT::OR,PVT::XOR,
        PVT::INDEX ) ) {
            if( def->modifier.is(PVT::PREFIX,PVT::SUFFIX,VT::DEFAULT,VT::DELETE) ) {
                success = false;
                diagnostics[def->getDocUri()]("99", def->modifier);
            }
            if( def->arg_names.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
    } else if( def->name.is(PVT::NOT,PVT::NEGATIVE,PVT::BITREV) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arg_names.size() != 1 or def->va_arg ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::INCREMENT,PVT::DECREMENT) ) {
        if( !def->modifier or def->modifier.is(PVT::REV,PVT::ISM,VT::DELETE,VT::DEFAULT) ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arg_names.size() != 0 or def->va_arg ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::ASSIGN,PVT::CCTOR,PVT::MCTOR) ) {
        if( def->modifier.is(VT::DEFAULT) ) {
            if( def->name.is(PVT::CCTOR,PVT::MCTOR ) ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
        } else if( def->modifier.is(VT::DELETE) ) {
            if(  def->name.is(PVT::ASSIGN) ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
        } else {
            if( def->modifier ) {
                success = false;
                diagnostics[def->getDocUri()]("99", def->modifier);
            }
            if( def->arg_names.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }  
        }
    } else if( def->name.is(PVT::ASSIGN_ADD,PVT::ASSIGN_SUB,PVT::ASSIGN_MUL,PVT::ASSIGN_DIV,PVT::ASSIGN_MOL,
        PVT::ASSIGN_SHL,PVT::ASSIGN_SHR,PVT::ASSIGN_BITAND,PVT::ASSIGN_BITOR,PVT::ASSIGN_BITXOR ) ) {
            if( def->modifier ) {
                success = false;
                diagnostics[def->getDocUri()]("99", def->modifier);
            }
            if( def->arg_names.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
    } else if( def->name.is(PVT::SCTOR,PVT::LCTOR) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
    } else if( def->name.is(PVT::DTOR) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arg_names.size() != 0 or def->va_arg ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::AS,PVT::MEMBER,PVT::ASPECT) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->name.is(PVT::MEMBER) and (def->arg_names.size() > 1 or def->va_arg) ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        } else if( def->name.is(PVT::AS) and (def->arg_names.size() != 0 or def->va_arg) ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::MOVE) ) {
        //应当允许重载两个move运算符，无参数版本用于move动作之后，带指针参数版本用于move动作之前
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arg_protos.size() == 1 ) {
            auto proto  =$(def->arg_protos[0]);
            if( proto ) {
                bool bad = false;
                if( proto->etype != eprototype::ptr ) bad = true;
                if( !proto->dtype->is_type(ConstraintedPointerType) ) bad = true;
                auto dt = ($typeexpr)proto->dtype->sub;
                if( dt ) {
                    if( !dt->is_type(StructType) ) bad = true;
                    if( dt->sub != def->getScope() ) bad = true;
                } else {
                    bad = true;
                }
                if( bad ) diagnostics[def->getDocUri()]("101", def->arg_names[0]);
            }
        } else if( def->arg_protos.size() > 1 ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else {
        return internal_error, false;
    }

    for( auto proto : def->arg_protos ) if( !$(proto) ) {
        success = false;
        diagnostics[proto->getDocUri()]("102", proto->phrase );
    }

    if( def->ret_proto and !$(def->ret_proto) ) {
        success = false;
        diagnostics[def->getDocUri()]("103", def->ret_proto->phrase);
    }

    return success;
}

bool SemanticContext::validateMethodImplementation( $metimpl impl ) {
    return true;
}

bool SemanticContext::validateOperatorImplementation( $opimpl impl ) {
    return true;
}

string SemanticContext::GetBinarySymbol( $node s ) {
    if( !s ) return "<error-0>";
    string symbol;
    auto mod = s->getModule();
    auto src = s;

    if( mod ) {
        if( mod->sctx.symbol_cache.count(s) )
            return mod->sctx.symbol_cache[s];
    }

    if( auto impl = ($implementation)s; impl ) {
        auto tc = GetThisClassDef(($node)impl);
        if( !tc ) return "<error-1>";
        symbol = tc->name.tostr();
        for( auto d = tc->getScope(); d != nullptr; d = d->getScope() ) {
            if( auto cd = ($classdef)d; cd ) symbol = (string)cd->name + "::" + symbol;
            else if( auto md = ($module)d; md ) symbol = (string)md->sig->name + "::" + symbol;
        }
        /** 接下来的工作在原型处处理 */
    } else if( auto def = ($definition)src; def ) {
        symbol = def->name.tostr();
        for( auto d = def->getScope(); d != nullptr; d = d->getScope() ) {
            if( auto cd = ($classdef)d; cd ) symbol = (string)cd->name + "::" + symbol;
            else if( auto md = ($module)d; md ) symbol = (string)md->sig->name + "::" + symbol;
        }
        if( auto cdef = ($classdef)def; cdef ) symbol = "class:" + symbol;
        else if( auto edef = ($enumdef)def; edef ) symbol = "enum:" + symbol;
        else if( auto mdef = ($metdef)def; mdef ) /** 此部分工作在原型部分完成 */;
        else if( auto odef = ($opdef)def; odef ) /** 此部分工作在原型部分完成 */;
        else if( auto adef = ($attrdef)def; adef ) symbol = "attribute:" + symbol;
        else if( auto idef = ($aliasdef)def; idef ) symbol = "alias:" + symbol;
        else return "<error-2>";

    } else if( auto proto = ($eprototype)src; proto ) {
        proto = $(proto);
        if( !proto ) return "<error-3>";
        switch( proto->etype ) {
            case eprototype::obj: symbol = "obj"; break;
            case eprototype::ptr: symbol = "ptr"; break;
            case eprototype::ref: symbol = "ref"; break;
            case eprototype::rel: symbol = "rel"; break;
            case eprototype::var: symbol = "var"; break;
            default: return "<error-4>";
        }
        symbol += ":" + GetBinarySymbol(($node)proto->dtype);
    } else if( auto type = ($typeexpr)src; type ) {
        type = $(type);
        if( !type ) return "<error-5>";
        if( type->is_type(UnknownType) ) {
            return "<unknown>";
        } else if( type->is_type(BasicTypeMask) ) {
            switch( type->id ) {
                case VoidType: symbol = "void"; break;
                case BooleanType: symbol = "boolean"; break;
                case Uint8Type: symbol = "uint8"; break;
                case Uint16Type: symbol = "uint16"; break;
                case Uint32Type: symbol = "uint32"; break;
                case Uint64Type: symbol = "uint64"; break;
                case Int8Type: symbol = "int8"; break;
                case Int16Type: symbol = "int16"; break;
                case Int32Type: symbol = "int32"; break;
                case Int64Type: symbol = "int64"; break;
                case Float32Type: symbol = "float32"; break;
                case Float64Type: symbol = "float64"; break;
                default: return "<error-6>";
            }
        } else if( type->is_type(ConstraintedPointerType) ) {
            symbol = "^" + GetBinarySymbol(($node)type->sub);
        } else if( type->is_type(UnconstraintedPointerType) ) {
            symbol = "*" + GetBinarySymbol(($node)type->sub);
        } else if( type->is_type(NullPointerType) ) {
            symbol = "null";
        } else if( type->is_type(CallableType) ) {
            /** 此处的任务在下面的call部分被处理 */
        } else if( type->is_type(EntityType) ) {
            symbol = "entity." + GetBinarySymbol(($node)type->sub);
        } else if( type->is_type(StructType) ) {
            symbol = "struct." + GetBinarySymbol(($node)type->sub);
        } else {
            return "<error-7>";
        }
    } else if( auto stmt = ($statement)src; stmt ) {
        symbol = stmt->name;
    }

    /** 处理方法原型和运算符原型 */
    if( auto met = dynamic_cast<metprototype*>(&*src); met ) {
        string prefix = "method";
        if( met->cons ) prefix += ".const";
        if( met->meta ) prefix += ".meta";
        symbol = prefix + ":" + symbol;
    } else if( auto op = dynamic_cast<opprototype*>(&*src); op ) {
        string prefix = "operator";
        if( op->cons ) prefix += ".const";
        if( op->modifier ) prefix += "." + (string)op->modifier;
        symbol = "operator:" + symbol;
        if( op->subtitle ) symbol += "." + (string)op->subtitle;
    }

    /** 处理可调用的情况 */
    auto call = ($callable_type)src;
    if( auto cal = dynamic_cast<callable*>(&*src); cal ) call = cal->getType();
    if( call ) {
        symbol += "(";
        for( int i = 0; i < call->arg_protos.size(); i++ ) {
            if( i != 0 ) symbol += ",";
            symbol += GetBinarySymbol(($node)call->arg_protos[i]);
        }
        if( call->va_arg ) {
            symbol += "...";
            if( call->va_arg.is(VT::L::LABEL) ) symbol += call->va_arg;
        }
        if( call->ret_proto ) symbol += "=>" + GetBinarySymbol(($node)call->ret_proto);
        symbol += ")";
        symbol += GetBinarySymbol(($node)call->ret_proto);
    }

    if( mod ) mod->sctx.symbol_cache[s] = symbol;
    return symbol;
}

$definition SemanticContext::GetDefinition( $implementation impl ) {
    if( !impl ) return nullptr;
    auto tc = GetThisClassDef(($node)impl);
    if( !tc ) return nullptr;

    auto symbol = GetBinarySymbol(($node)impl);
    for( auto def : tc->contents ) {
        if( auto met = ($metdef)def; met ) {
            auto defsymbol = GetBinarySymbol(($node)met);
            if( defsymbol == symbol ) return def;
        } else if( auto op = ($opdef)def; op ) {
            auto defsymbol = GetBinarySymbol(($node)op);
            if( defsymbol == symbol ) return def;
        }
    }

    auto module = impl->getModule();
    auto& diagnostics = module->sctx.diagnostics;
    diagnostics[impl->getDocUri()]("96", impl->name);
    return nullptr;
}

$classdef SemanticContext::GetThisClassDef( $node n ) {
    while( n and !n->is(node::IMPLEMENTATION) and !n->is(node::CLASSDEF) ) n = n->getScope();
    if( auto impl = ($implementation)n; impl ) {
        return ReachClass(impl->host);
    } else if( auto def = ($classdef)n; def ) {
        return def;
    } else {
        return nullptr;
    }
}

everything SemanticContext::Reach( $nameexpr name, SearchOptions opts, $scope scope, aliasdefs paddings ) {
    if( !name ) return nothing;
    if( !scope ) scope = name->getScope();
    while( scope and (!scope->isscope() or scope->is(node::FRAGMENT)) ) scope = scope->getScope();
    if( !scope or !scope->isscope() ) return nothing;

    auto module = scope->getModule();
    auto& semantic = module->sctx;
    auto& diagnostics = semantic.diagnostics;
    everything results;

    if( auto sc = ($module)scope; sc ) {
        /** 尝试匹配自身 */
        if( name->name == sc->sig->name ) results << (anything)sc;

        /** 搜索内部定义 */
        for( auto def : sc->defs )
            if( def->name == name->name ) results << (anything)def;

        /** 搜索联合定义 */
        for( auto dep : sc->sig->deps ) {
            if( !dep->alias.is(VT::L::THIS) ) continue;
            auto mod = semantic.getModule(dep);
            if( !mod ) return internal_error, nothing;
            for( auto def : mod->defs )
                if( def->name == name->name ) results << (anything)def;
        }
        
        /** 搜索依赖 */
        for( auto dep : sc->sig->deps ) {
            if( dep->alias.is(VT::L::THIS) ) continue;
            bool found = false;
            if( dep->alias and dep->alias == name->name ) found = true;
            else if( dep->name == name->name ) found = true;
            if( !found ) continue;
            auto mod = semantic.getModule(dep);
            if( !mod ) return internal_error, nothing;
            results << (anything)mod;
        }
    } else if( auto sc = ($classdef)scope; sc ) {
        /** 搜索类的内部成员和内部定义 */
        for( auto def : sc->contents ) {
            if( auto attr = ($attrdef)def; attr ) {
                if( (opts & SearchOption::MEMBERS) == 0 ) continue;
                if( attr->name == name->name ) results << (anything)attr;
            } else {
                if( (opts & SearchOption::INNERS) == 0 ) continue;
                if( def->name == name->name ) results << (anything)def;
            }
        }
        
        /** 搜索模板参数 */
        if( sc->targs.size() ) for( auto i = 0; i < sc->targf.size(); i++ ) {
            if( sc->targf[i] == name->name ) {
                results << (anything)sc->targs[i];
            }
        }
    } else if( auto sc = ($implementation)scope; sc ) {
        for( auto arg : sc->args )
            if( arg->name == name->name )
                results << (anything)arg;
    } else if( auto sc = ($blockstmt)scope; sc ) {
        for( auto stmt : *sc )
            if( stmt->name == name->name )
                results << (anything)stmt;
    } else if( auto sc = ($loopstmt)scope; sc ) {
        if( sc->it->name == name->name )
            results << (anything)sc->it;
    } else if( auto sc = ($assumestmt)scope; sc ) {
        if( sc->variable->name == name->name )
            results << (anything)sc->variable;
    } else if( auto sc = ($lambdaexpr)scope; sc ) {
        for( auto arg : sc->args )
            if( arg->name == name->name )
                results << (anything)arg;
    } else {
        return internal_error, nothing;
    }

    /** 处理模板类用例的情况 */
    if( name->targs.size() ) {
        if( results.size() != 1 ) return diagnostics[name->getDocUri()]("90", name->name), nothing;
        auto def = ($classdef)results[0];
        if( !def ) return diagnostics[name->getDocUri()]("90", name->name), nothing;
        auto usage = GetTemplateUsage( def, name->targs );
        if( !usage ) return nothing;
        results[0] = usage;
    }

    if( results.size() != 0 and name->next ) {
        /** 处理作用域深入的情况 */
        if( results.size() != 1 )
            return diagnostics[name->getDocUri()]("88", name->name), nothing;
        /** 处理目标并非作用域的情况 */
        auto sc = ($node)results[0];
        if( !sc or !sc->isscope() )
            return diagnostics[name->getDocUri()]("89", name->name), nothing;
        return Reach( name->next, SearchOption::ANY, sc, paddings );
    } else if( results.size() == 0 ) {
        /** 处理当前作用域没有搜索到当前目标 */
        if( auto sc = ($module)scope; sc ) {
            //[Nothing to be done]
        } if( auto sc = ($classdef)scope; sc ) {
            /** 当前作用域没有目标，尝试搜索基类 */
            if( opts & SearchOption::SUPER )
                for( auto super : sc->supers ) {
                    auto temp = Reach(super, SearchOption::ALL|SearchOption::ANY, super->getScope(), paddings );
                    if( temp.size() != 1 ) continue; // 基类不唯一或不可达的错误在类语义检查中被报告
                    auto def = ($classdef)temp[0];
                    if( !def ) continue; // 基类不可达的错误在语义分析中报告
                    results += Reach(name, SearchOption::SUPER|SearchOption::MEMBERS, def, paddings );
                }
            /** 若当前作用域没有目标，尝试搜索父作用域 */
            if( opts & SearchOption::PARENT ) {
                results += Reach(name, SearchOption::PARENT|SearchOption::INNERS, sc->getScope(), paddings);
            }
        } else if( auto sc = ($implementation)scope; sc ) {
            if( auto def = GetDefinition(sc); def and (SearchOption::PARENT&opts) ) {
                results += Reach(name, SearchOption::ANY|SearchOption::ALL, def, paddings);
            }
        } else if( (opts & SearchOption::PARENT) and scope->getScope() ) {
            results += Reach(name, SearchOption::ALL|SearchOption::ANY, scope->getScope(), paddings );
        } else {
            //[Nothing to be done]
        }
    }

    /** 处理别名，将搜索到的别名解析 */
    for( auto i = 0; i < results.size(); i++ ) if( auto alias = ($aliasdef)results[i]; alias ) {

        /** 处理循环引用的情况 */
        for( auto padding : paddings ) if( padding == alias ) {
            diagnostics[alias->getDocUri()]("87", alias->phrase);
            return nothing;
        }

        /** 解析别名引用 */
        auto res = Reach(alias->tagret, 
            SearchOption::ALL|SearchOption::ANY, 
            alias->getScope(), 
            paddings+aliasdefs{alias} );
        if( res.size() == 0 ) return nothing;
        results += res;
        results.remove(i--);
    }

    return results;
}

$classdef SemanticContext::ReachClass( $nameexpr name, SearchOptions opts, $scope scope, aliasdefs paddings ) {
    auto res = Reach(name, opts, scope, paddings);
    if( res.size() != 1 ) return nullptr;
    return ($classdef)res[0];
}

everything SemanticContext::$( $nameexpr name, SearchOptions opts, $scope scope, aliasdefs paddings ) {
    return Reach( name, opts, scope, paddings );
}

$eprototype SemanticContext::ReductPrototype( $eprototype proto ) {
    if( !proto ) return nullptr;
    if( !$(proto->dtype, proto) ) return nullptr;
    if( proto->etype == eprototype::var ) {
        if( proto->dtype->is_type(UnknownType) ) proto->etype = eprototype::var;
        else if( proto->dtype->is_type(PointerTypeMask) ) proto->etype = eprototype::ptr;
        else proto->etype = eprototype::obj;
    }
    return proto;
}

$eprototype SemanticContext::$( $eprototype proto ) {
    return ReductPrototype(proto);
}

$typeexpr SemanticContext::ReductTypeexpr( $typeexpr type, $eprototype proto ) {
    auto& diagnostics = type->getModule()->sctx.diagnostics;
    if( type->is_type(NamedType) ) {
        auto res = $(($nameexpr)type->sub);
        if( res.size() != 1 ) {
            diagnostics[type->getDocUri()];
            if( res.size() == 0 ) diagnostics("97", type->phrase);
            else diagnostics("93", type->phrase);
            type->id = UnsolvableType;
            return nullptr;
        }
        if( auto def = ($classdef)res[0]; def ) {
            type->id = StructType;
            type->sub = def;
        } else if( auto targ = ($eprototype)res[0]; targ ) {
            if( proto ) if( proto->etype == eprototype::var ) proto->etype = targ->etype;
            auto t = ReductTypeexpr(targ->dtype, nullptr); // 通过特定的源代码书写，可能造成编译器无限递归而崩溃
            if( !t ) {
                type->id = UnsolvableType;
                return nullptr;
            } else {
                type->id = t->id;
                type->sub = t->sub;
            }
        } else {
            diagnostics[type->getDocUri()]("94", type->phrase);
            return nullptr;
        }
    } else if( type->is_type(ThisClassType) ) {
        auto def = GetThisClassDef(($node)type);
        if( !def ) {
            type->id = UnsolvableType;
            diagnostics[type->getDocUri()]("95", type->phrase);
            return nullptr;
        } else {
            type->id = StructType;
            type->sub = def;
        }
    } else if( type->is_type(UnsolvableType) ) {
        return nullptr;
    } else if( auto sub = ($typeexpr)type->sub; sub ) {
        if( !$(sub) ) return nullptr;
    }

    return type;
}

$typeexpr SemanticContext::$( $typeexpr type, $eprototype proto ) {
    return ReductTypeexpr(type, proto);
}

bool SemanticContext::CanBeInstanced( $classdef def ) {
    if( def->targf.size() and def->targs.size() == 0 ) return false;
    /**[TODO] 检查抽象类所有方法被实现的情况 */
    return true;
}

$classdef SemanticContext::GetTemplateUsage( $classdef def, eprototypes targs ) {
    return nullptr; //[TODO]
}

}

#endif