#ifndef __semantic_cpp__
#define __semantic_cpp__

#include "semantic.hpp"

namespace alioth {

module::module( SemanticContext& context ):sctx(context) {

}

bool module::is( type t )const {
    return t == MODULE;
}

$node module::clone( $scope scope ) const {
    throw logic_error("modle::clone(): method not allowed");
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

    definitions defs;

    for( auto [_,rec] : sig->docs ) {
        auto fg = rec.fg;
        defs += fg->defs;
        mod->impls += fg->impls;
        fg->setScope(mod);
    }

    bool success = true;
    for( int i = 0; i < defs.size(); i++ ) {
        auto& def = defs[i];
        if( auto cdef = ($classdef)def; cdef and (string)cdef->name == (string)mod->sig->name ) {
            if( mod->trans )
                success = (diagnostics
                    [cdef->getDocUri()]("77", cdef->name)
                    [-1](mod->trans->getDocUri(),"45",mod->trans->name), false);
            if( cdef->targf.size() )
                success = (diagnostics[cdef->getDocUri()]("74", cdef->name), false);
            if( cdef->abstract )
                success = (diagnostics[cdef->getDocUri()]("75", cdef->name), false);
            if( cdef->supers.size() )
                success = (diagnostics[cdef->getDocUri()]("76", cdef->name), false);
            if( success ) {
                mod->trans = def;
                defs.remove(i--);
                cdef->defs += defs;
            }
        }
    }
    if( !mod->trans ) {
        mod->trans = new classdef;
        mod->trans->setScope(mod);
        mod->trans->name = mod->sig->name;
        mod->trans->defs = defs;
    }

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

void SemanticContext::clearCache() {
    symbol_cache.clear();
    dep_cache.clear();
    searching_layers.clear();
    alias_searching_layers.clear();
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
    for( auto& def : mod->trans->defs ) {

        if( auto cldef = ($classdef)def; cldef ) success = validateClassDefinition(cldef) and success;
        else if( auto endef = ($enumdef)def; endef ) success =  validateEnumDefinition( endef ) and success;
        else if( auto aldef = ($aliasdef)def; aldef ) success =  validateAliasDefinition( aldef ) and success;
        else if( auto atdef = ($attrdef)def; atdef ) success =  validateAttributeDefinition( atdef ) and success;
        else if( auto medef = ($metdef)def; medef ) success =  validateMethodDefinition( medef ) and success;
        else if( auto opdef = ($opdef)def; opdef ) success =  validateOperatorDefinition( opdef ) and success;
        else internal_error, success = false;

        /** 检查重复定义 */
        for( auto& prv : mod->trans->defs ) {
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

        if( mod->sig->entry ) {
            auto i32 = eprototype::make(mod,token("int32"), 
                typeexpr::make(mod,token("int32"),Int32Type));
            auto pp8 = eprototype::make(mod,token("**int8"), 
                typeexpr::make(mod,token("**int8"), UnconstraintedPointerType,
                    (anything)typeexpr::make(mod,token("**int8"), UnconstraintedPointerType,
                        (anything)typeexpr::make(mod,token("**int8"), Int8Type))));
            if( auto met = ($metdef)def; met and met->name == mod->sig->entry.mark ) {
                if( met->arguments.size() != 2 ) continue;
                if( !IsIdentical(met->ret_proto,i32) ) continue;
                if( !IsIdentical(met->arguments[0]->proto,i32) ) continue;
                if( !IsIdentical(met->arguments[1]->proto,pp8) ) continue;
                mod->entry = met;
            }
        }
    }

    if( mod->sig->entry and !mod->entry ) {
        auto& space = mod->getCompilerContext().getSpaceEngine();
        diagnostics[space.getUri(mod->sig->entry.doc)]("116", mod->sig->entry.mark);
        success = false;
    }

    return success;
}

bool SemanticContext::validateClassDefinition(  $classdef cls ) {
    bool success = true;

    /** 为剩余部分忽略模板类,只有模板类用例才会被检查 */
    if( cls->targf.size() ) {
        /** 模板类需要检查谓词参数 */
        for( auto pred : cls->preds )
            for( auto prei : pred ) if( prei.type ) {
                if( !$(prei.type) ) {
                    success = false;
                    diagnostics[cls->getDocUri()]("107", prei.vn);
                } else if( prei.type->is_type(PointerTypeMask) ) {
                    success = false;
                    diagnostics[cls->getDocUri()]("108", prei.vn);
                }
            }
        if( !cls->targs.size() ) return success;
    }

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
    for( auto& def : cls->defs ) {

        if( auto cldef = ($classdef)def; cldef ) success = validateClassDefinition(cldef) and success;
        else if( auto endef = ($enumdef)def; endef ) success =  validateEnumDefinition( endef ) and success;
        else if( auto aldef = ($aliasdef)def; aldef ) success =  validateAliasDefinition( aldef ) and success;
        else if( auto atdef = ($attrdef)def; atdef ) success =  validateAttributeDefinition( atdef ) and success;
        else if( auto medef = ($metdef)def; medef ) success =  validateMethodDefinition( medef ) and success;
        else if( auto opdef = ($opdef)def; opdef ) success =  validateOperatorDefinition( opdef ) and success;
        else internal_error, success = false;

        /** 检查重复定义 */
        for( auto& prv : cls->defs ) {
            if( &def == &prv ) break;
            if( def->name != prv->name ) continue;
            bool repeat = true;

            string dname = GetBinarySymbol(($node)def);
            string pname = GetBinarySymbol(($node)prv);
            if( def->is(node::METHODDEF) and prv->is(node::METHODDEF) ) {
                repeat = (dname == pname);
            } else if( def->is(node::OPERATORDEF) and prv->is(node::OPERATORDEF) ) {
                auto odef = ($opdef)def, oprv = ($opdef)prv;
                if( !odef->modifier.is(VT::DELETE) and !oprv->modifier.is(VT::DELETE) )
                    repeat = (dname == pname);
            }

            if( repeat ) {
                diagnostics[def->getDocUri()]("98", def->name, dname)
                [-1](prv->getDocUri(), "45", prv->name);
                success = false;
            }
        }
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
            for( auto content : def->defs ) {
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

    auto result = $(def->target);

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
    for( auto arg : def->arguments ) {
        if( !$(arg->proto) ) success = false;
    }
    if( !$(def->ret_proto) ) success = false;
    return success;
}

bool SemanticContext::validateOperatorDefinition(  $opdef def ) {
    bool success = true;

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
            if( def->arguments.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
    } else if( def->name.is(PVT::NOT,PVT::NEGATIVE,PVT::BITREV) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arguments.size() != 1 or def->va_arg ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::INCREMENT,PVT::DECREMENT) ) {
        if( !def->modifier or def->modifier.is(PVT::REV,PVT::ISM,VT::DELETE,VT::DEFAULT) ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arguments.size() != 0 or def->va_arg ) {
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
            if( def->arguments.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            } else if( auto proto = $(def->arguments[0]->proto); proto ) {
                if( !proto->dtype->is_type(StructType) or proto->dtype->sub != def->getScope() ) {
                    diagnostics[def->getDocUri()]("101", proto->dtype->phrase);
                    success = false;
                } else if( def->name.is(PVT::CCTOR) ) {
                    if( proto->etype != eprototype::obj or !proto->cons ) {
                        diagnostics[def->getDocUri()]("101", proto->phrase );
                        success = false;
                    }
                } else if( def->name.is(PVT::MCTOR) ) {
                    if( proto->etype != eprototype::rel or proto->cons ) {
                        diagnostics[def->getDocUri()]("101", proto->phrase );
                        success = false;
                    }
                }
            }
        }
    } else if( def->name.is(PVT::ASSIGN_ADD,PVT::ASSIGN_SUB,PVT::ASSIGN_MUL,PVT::ASSIGN_DIV,PVT::ASSIGN_MOL,
        PVT::ASSIGN_SHL,PVT::ASSIGN_SHR,PVT::ASSIGN_BITAND,PVT::ASSIGN_BITOR,PVT::ASSIGN_BITXOR ) ) {
            if( def->modifier ) {
                success = false;
                diagnostics[def->getDocUri()]("99", def->modifier);
            }
            if( def->arguments.size() != 1 or def->va_arg ) {
                success = false;
                diagnostics[def->getDocUri()]("100", def->name);
            }
    } else if( def->name.is(PVT::SCTOR,PVT::LCTOR) ) {
        if( def->modifier and !def->modifier.is(VT::DEFAULT) ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
    } else if( def->name.is(PVT::DTOR) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arguments.size() != 0 or def->va_arg ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::AS,PVT::MEMBER,PVT::ASPECT) ) {
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->name.is(PVT::MEMBER) and (def->arguments.size() > 1 or def->va_arg) ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        } else if( def->name.is(PVT::AS) and (def->arguments.size() != 0 or def->va_arg) ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else if( def->name.is(PVT::MOVE) ) {
        //应当允许重载两个move运算符，无参数版本用于move动作之后，带指针参数版本用于move动作之前
        if( def->modifier ) {
            success = false;
            diagnostics[def->getDocUri()]("99", def->modifier);
        }
        if( def->arguments.size() == 1 ) {
            auto proto  =$(def->arguments[0]->proto);
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
                if( bad ) diagnostics[def->getDocUri()]("101", def->arguments[0]->phrase);
            }
        } else if( def->arguments.size() > 1 ) {
            success = false;
            diagnostics[def->getDocUri()]("100", def->name);
        }
    } else {
        return internal_error, false;
    }

    for( auto arg : def->arguments ) if( !$(arg->proto) ) {
        success = false;
        diagnostics[arg->proto->getDocUri()]("102", arg->proto->phrase );
    }

    if( def->ret_proto and !$(def->ret_proto) ) {
        success = false;
        diagnostics[def->getDocUri()]("103", def->ret_proto->phrase);
    }

    return success;
}

bool SemanticContext::validateMethodImplementation( $metimpl impl ) {
    bool success = true;

    auto host = ReachClass(impl->host);
    if( !host ) {
        diagnostics[impl->getDocUri()]("112", impl->host->phrase, impl->name);
        return false;
    }

    $metdef org;
    for( auto content : host->defs ) if( auto def = ($metdef)content; def ) {
        if( def->name != impl->name ) continue;
        if( def->arguments.size() != impl->arguments.size() ) continue;
        if( (bool)def->cons xor (bool)impl->cons ) continue;
        if( (bool)def->meta xor (bool)impl->meta ) continue;
        if( !IsIdentical(def->ret_proto,impl->ret_proto,true) ) continue;
        bool dif = false;
        for( auto i = 0; i < def->arguments.size(); i++ )
            if( !IsIdentical(def->arguments[i]->proto,impl->arguments[i]->proto, true) ) {
                dif = true;
                break;
            }
        if( dif ) continue;
        org = def;
        break;
    } if( !org ) {
        diagnostics[impl->getDocUri()]("113", impl->name);
        return false;
    }

    return success;
}

bool SemanticContext::validateOperatorImplementation( $opimpl impl ) {
    bool success = true;

    auto host = ReachClass(impl->host);
    if( !host ) {
        diagnostics[impl->getDocUri()]("112", impl->host->phrase, impl->name);
        return false;
    }

    $opdef org;
    for( auto content : host->defs ) if( auto def = ($opdef)content; def ) {
        if( def->name.tostr() != impl->name.tostr() ) continue;
        if( def->subtitle != impl->subtitle ) continue;
        if( def->arguments.size() != impl->arguments.size() ) continue;
        if( (bool)def->cons xor (bool)impl->cons ) continue;
        if( def->modifier != impl->modifier ) continue;
        if( (bool)def->ret_proto xor (bool)impl->ret_proto ) continue;
        if( !IsIdentical(def->ret_proto,impl->ret_proto,true) ) continue;
        bool dif = false;
        for( auto i = 0; i < def->arguments.size(); i++ )
            if( !IsIdentical(def->arguments[i]->proto,impl->arguments[i]->proto, true) ) {
                dif = true;
                break;
            }
        if( dif ) continue;
        org = def;
        break;
    } if( !org ) {
        diagnostics[impl->getDocUri()]("113", impl->name);
        return false;
    }

    return true;
}

string SemanticContext::GetBinarySymbol( $node s ) {
    if( !s ) return "<error-0>";
    string symbol;
    auto mod = s->getModule();
    auto src = s;
    bool skip = false;

    if( mod ) {
        if( mod->sctx.symbol_cache.count(s) )
            return mod->sctx.symbol_cache[s];
    }

    if( auto impl = ($implementation)s; impl ) {
        if( auto def = GetDefinition(impl); def ) {
            src = def;
        } else {
            auto tc = GetThisClassDef(($node)impl);
            if( !tc ) return "<error-1>";
            symbol = tc->name.tostr();
            for( auto d = tc->getScope(); d != nullptr; d = d->getScope() ) {
                if( auto cd = ($classdef)d; cd ) symbol = (string)cd->name + "::" + symbol;
                else if( auto md = ($module)d; md ) symbol = (string)md->sig->name + "::" + symbol;
            }
        }
        /** 接下来的工作在原型处处理 */
    }
    if( auto met = ($metdef)src; met and met->raw ) {
        auto [suc,dat,diag] = met->raw.extractContent();
        if( !suc ) return "<error-8>";
        symbol = dat;
        skip = true;
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

    if( !skip ) {
        /** 处理模板类的情况 */
        if( auto usage = ($classdef)src; usage and usage->targs.size() ) {
            symbol += "<";
            for( int i = 0; i < usage->targs.size(); i++ ) {
                if( i != 0 ) symbol += ",";
                symbol += GetBinarySymbol(($node)usage->targs[i]);
            }
            symbol += ">";
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
            if( op->modifier and !op->modifier.is(VT::DEFAULT) ) prefix += "." + (string)op->modifier;
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
    }

    if( mod ) mod->sctx.symbol_cache[s] = symbol;
    return symbol;
}

$definition SemanticContext::GetDefinition( $implementation impl ) {
    if( !impl ) return nullptr;
    auto tc = GetThisClassDef(($node)impl);
    if( !tc ) return nullptr;

    for( auto def : tc->defs ) {
        if( !def->is(node::METHODDEF) and !def->is(node::OPERATORDEF) ) continue;
        if( impl->name.tostr() != def->name.tostr() ) continue;
        if( auto metd = ($metdef)def; metd ) {
            auto meti = ($metimpl)impl;
            if( (bool)metd->cons xor (bool)meti->cons ) continue;
            if( (bool)metd->meta xor (bool)meti->meta ) continue;
            if( metd->arguments.size() != meti->arguments.size() ) continue;
            for( auto i = 0; i < metd->arguments.size(); i++ )
                if( !IsIdentical(metd->arguments[i]->proto, meti->arguments[i]->proto) ) continue;
            if( !IsIdentical(metd->ret_proto,meti->ret_proto) ) continue;
        } else if( auto opd = ($opdef)def; opd ) {
            auto opi = ($opimpl)impl;
            if( opd->modifier != opi->modifier ) continue;
            if( (bool)opd->cons xor (bool)opi->cons ) continue;
            if( opd->subtitle != opi->subtitle ) continue;
            if( opd->arguments.size() != opi->arguments.size() ) continue;
            for( auto i = 0; i < opd->arguments.size(); i++ )
                if( !IsIdentical(opd->arguments[i]->proto, opi->arguments[i]->proto) ) continue;
            if( (bool)opd->ret_proto xor (bool)opi->ret_proto ) continue;
            if( !IsIdentical(opd->ret_proto,opi->ret_proto) ) continue;
        }
    }

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

everything SemanticContext::Reach( $nameexpr name, SearchOptions opts, $scope scope ) {
    if( !name ) return nothing;
    if( !scope ) scope = name->getScope();
    while( scope and (!scope->isscope() or scope->is(node::FRAGMENT)) ) scope = scope->getScope();
    if( !scope or !scope->isscope() ) return nothing;
    auto layer = searching_layer(scope, name);
    if( !layer ) return nothing;

    auto module = scope->getModule();
    auto& semantic = module->sctx;
    auto& diagnostics = semantic.diagnostics;
    everything results;

    if( auto sc = ($module)scope; sc ) {
        /** 尝试匹配自身 */
        if( name->name == sc->trans->name ) results << (anything)sc->trans;

        /** 搜索内部定义 */
        for( auto def : sc->trans->defs )
            if( def->name == name->name ) results << (anything)def;
            else if( auto enm = ($enumdef)def; enm )
                for( auto item : enm->items ) if( item->name == name->name ) results << (anything)item;

        /** 搜索联合定义 */
        for( auto dep : sc->sig->deps ) {
            if( !dep->alias.is(VT::L::THIS) ) continue;
            auto mod = semantic.getModule(dep);
            if( !mod ) return internal_error, nothing;
            for( auto def : mod->trans->defs )
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
            results << (anything)mod->trans;
        }
    } else if( auto sc = ($classdef)scope; sc ) {
        /** 搜索类的内部成员和内部定义 */
        for( auto def : sc->defs ) {
            if( auto attr = ($attrdef)def; attr ) {
                if( (opts & SearchOption::MEMBERS) == 0 ) continue;
                if( attr->name == name->name ) results << (anything)attr;
            } else {
                if( (opts & SearchOption::INNERS) == 0 ) continue;
                if( def->name == name->name ) results << (anything)def;
                else if( auto enm = ($enumdef)def; enm )
                    for( auto item : enm->items ) if( item->name == name->name ) results << (anything)item;
            }
        }
        
        /** 搜索模板参数 */
        if( sc->targs.size() ) for( auto i = 0; i < sc->targf.size(); i++ ) {
            if( sc->targf[i] == name->name ) {
                results << (anything)sc->targs[i];
            }
        }
    } else if( auto sc = ($metimpl)scope; sc ) {
        for( auto arg : sc->arguments )
            if( arg->name == name->name )
                results << (anything)arg;
    } else if( auto sc = ($opimpl)scope; sc ) {
        for( auto arg : sc->arguments )
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
        for( auto arg : sc->arguments )
            if( arg->name == name->name )
                results << (anything)arg;
    } else {
        return internal_error, nothing;
    }

    /** 处理别名，将搜索到的别名解析 */
    for( auto i = 0; i < results.size(); i++ ) if( auto alias = ($aliasdef)results[i]; alias ) {

        /** 检查别名循环 */
        for( auto layer : semantic.alias_searching_layers ) if( layer == alias ) {
            diagnostics[alias->getDocUri()]("87", alias->phrase);
            return nothing;
        }

        /** 解析别名引用 */
        semantic.alias_searching_layers.push(alias);
        auto res = Reach(alias->target, SearchOption::ALL|SearchOption::ANY, alias->getScope() );
        semantic.alias_searching_layers.pop();
        if( res.size() == 0 ) return nothing;
        results += res;
        results.remove(i--);
    }

    /** 处理模板类用例的情况 */
    if( name->targs.size() and results.size() ) {
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
        /** 深入作用域搜索 */
        return Reach( name->next, SearchOption::ANY, sc );
    } else if( results.size() == 0 ) {
        /** 处理当前作用域没有搜索到当前目标 */
        if( auto sc = ($module)scope; sc ) {
            //[Nothing to be done]
        } if( auto sc = ($classdef)scope; sc ) {
            /** 当前作用域没有目标，尝试搜索基类 */
            if( opts & SearchOption::SUPER )
                for( auto super : sc->supers ) {
                    auto temp = Reach(super, SearchOption::ALL|SearchOption::ANY, super->getScope() );
                    if( temp.size() != 1 ) continue; // 基类不唯一或不可达的错误在类语义检查中被报告
                    auto def = ($classdef)temp[0];
                    if( !def ) continue; // 基类不可达的错误在语义分析中报告
                    results += Reach(name, SearchOption::SUPER|SearchOption::MEMBERS, def );
                }
            /** 若当前作用域没有目标，尝试搜索父作用域 */
            if( (opts & SearchOption::PARENT) and sc->getScope() ) {
                results += Reach(name, SearchOption::PARENT|SearchOption::INNERS, sc->getScope());
            }
        } else if( auto sc = ($implementation)scope; sc ) {
            if( auto def = GetThisClassDef(($node)sc); def and (SearchOption::PARENT&opts) ) {
                results += Reach(name, SearchOption::ANY|SearchOption::ALL, def);
            }
        } else if( (opts & SearchOption::PARENT) and scope->getScope() ) {
            results += Reach(name, SearchOption::ALL|SearchOption::ANY, scope->getScope() );
        } else {
            //[Nothing to be done]
        }
    }

    return results;
}

everything SemanticContext::$( $nameexpr name, SearchOptions opts, $scope scope ) {
    return Reach( name, opts, scope );
}

$classdef SemanticContext::ReachClass( $nameexpr name, SearchOptions opts, $scope scope ) {
    auto res = Reach(name, opts, scope);
    if( res.size() != 1 ) return nullptr;
    return ($classdef)res[0];
}

$eprototype SemanticContext::ReductPrototype( $eprototype proto ) {
    if( !proto ) return nullptr;
    if( !$(proto->dtype, proto) ) return nullptr;
    if( proto->etype == eprototype::var ) {
        if( proto->dtype->is_type(UnknownType) ) proto->etype = eprototype::var;
        else if( proto->dtype->is_type(PointerTypeMask) ) proto->etype = eprototype::ptr;
        else proto->etype = eprototype::obj;
    }

    if( proto->etype == eprototype::obj and proto->dtype->is_type(StructType) and !CanBeInstanced(($classdef)proto->dtype->sub) ) {
        auto& diagnostics = proto->getModule()->sctx.diagnostics;
        diagnostics[proto->getDocUri()]("115", proto->dtype->phrase);
        return nullptr;
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
        } else if( auto def = ($enumdef)res[0]; def ) {
            type->id = EnumType;
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
    } else if( type->is_type(CallableType) ) {
        auto success = true;
        auto call = ($callable_type)type->sub;
        for( auto arg : call->arg_protos )
            if( !$(arg) ) success = false;
        if( !$(call->ret_proto) ) success = false;
        if( !success ) return nullptr;
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
    if( def->abstract ) return false;
    return true;
}

$classdef SemanticContext::GetTemplateUsage( $classdef def, eprototypes targs ) {
    auto module = def->getModule();
    auto& context = module->sctx;
    auto& diagnostics = context.diagnostics;

    /** 检查各种类型的内部错误 */
    if( def->targf.size() == 0 ) return internal_error, nullptr;
    else if( targs.size() == 0 ) return internal_error, nullptr;
    else if( def->targs.size() != 0 ) return internal_error, nullptr;

    /** 检查模板参数列表 */
    if( targs.size() != def->targf.size() ) {
        return diagnostics[targs[-1]->getDocUri()]("104", targs[-1]->phrase), nullptr;
    } else {
        bool success = true;
        for( auto targ : targs ) if( !$(targ) ) {
            success = false;
            diagnostics[targ->getDocUri()]("110", targ->phrase);
        }
        if( !success ) return nullptr;
    }

    /** 检查已经存在的模板用例 */
    for( auto usage : def->usages ) {
        bool same = true;
        for( auto i = 0; i < targs.size(); i++ ) {
            if( !IsIdentical(usage->targs[i], targs[i]) ) {
                same = false;
                break;
            }
        }
        if( same ) return usage;
    }

    /** 产生模板用例 */
    auto usage = ($classdef)def->clone(def->getScope());
    usage->targs = targs;

    /** 检查谓词条件 */
    set<int> premise;
    for( int i = 0; i < def->preds.size(); i++ ) {
        auto pred = def->preds[i];
        bool verdict = true;
        for( auto pi : pred ) {
            $eprototype targ;
            for( auto i = 0; i < def->targf.size(); i++ ) if( def->targf[i] == pi.targ ) targ = targs[i];
            if( !targ ) return internal_error, nullptr;
            switch( pi.rule ) {
                case 1: if( targ->etype == eprototype::obj) verdict = false; break;
                case 2: if( targ->etype == eprototype::ptr) verdict = false; break;
                case 3: if( targ->etype == eprototype::ref) verdict = false; break;
                case 4: if( targ->etype == eprototype::rel) verdict = false; break;
                case 5: if( targ->etype != eprototype::obj) verdict = false; break;
                case 6: if( targ->etype != eprototype::ptr) verdict = false; break;
                case 7: if( targ->etype != eprototype::ref) verdict = false; break;
                case 8: if( targ->etype != eprototype::rel) verdict = false; break;
                case 9: if( targ->dtype->is_type(PointerTypeMask) ) verdict = false; break;
                case 10:if( !targ->dtype->is_type(PointerTypeMask) ) verdict = false; break;
                case 11: {
                    if( !$(pi.type) or !pi.type->is_type(StructType) or !targ->dtype->is_type(StructType) ) break;
                    auto table = GetInheritTable(($classdef)targ->dtype->sub) << ($classdef)targ->dtype->sub;
                    for( auto super : table ) {
                        if( pi.type->sub == super ) {verdict = false; break;}
                    }
                } break;
            }
        }
        if( verdict ) premise.insert(i);
    }
    if( def->preds.size() and premise.empty() )
        return diagnostics[targs[0]->getDocUri()]("109", targs[0]->phrase, targs[-1]->phrase), nullptr;

    /** 根据谓词删除前提不成立的定义 */
    for( auto i = 0; i < usage->defs.size(); i++ ) {
        auto content = usage->defs[i];
        if( content->premise.size() ) {
            bool found = false;
            for( auto i : content->premise ) if( premise.count(i) ){ found = true; break;}
            if( !found ) usage->defs.remove(i--);
        }
    }

    if( !context.validateClassDefinition(usage) ) return nullptr;
    def->usages << usage;

    return usage;
}

bool SemanticContext::IsIdentical( $eprototype a, $eprototype b, bool u ) {
    a = $(a);
    b = $(b);
    if( !a or !b ) return false;
    if( a->etype != b->etype ) return false;
    if( (bool)a->cons xor (bool)b->cons ) return false;
    return IsIdentical( a->dtype, b->dtype, u );
}

bool SemanticContext::IsIdentical( $typeexpr a, $typeexpr b, bool u ) {
    a = $(a);
    b = $(b);
    if( !a or !b ) return false;
    if( !u and (a->is_type(UnknownType) or b->is_type(UnknownType)) ) return false;
    if( a->id != b->id ) return false;
    if( a->is_type(StructType) or a->is_type(EntityType) ) {
        return a->sub == b->sub;
    } else if( a->is_type(PointerTypeMask) ) {
        return IsIdentical( ($typeexpr)a->sub, ($typeexpr)b->sub );
    } else if( a->is_type(CallableType) ) {
        auto calla = ($callable_type)a->sub;
        auto callb = ($callable_type)b->sub;
        if( !IsIdentical(calla->ret_proto,callb->ret_proto, true) ) return false;
        if( calla->arg_protos.size() != callb->arg_protos.size() ) return false;
        if( (bool)calla->va_arg xor (bool)callb->va_arg ) return false;
        for( auto i = 0; i < calla->arg_protos.size(); i++ )
            if( !IsIdentical(calla->arg_protos[i], callb->arg_protos[i], true) )
                return false;
        return true;
    } else {
        return true;
    }
}

classdefs SemanticContext::GetInheritTable( $classdef def, classdefs paddings ) {
    classdefs table;
    
    for( auto padding : paddings ) if( def == padding ) return {};

    for( auto super : def->supers ) {
        auto res = $(super);
        if( res.size() != 1 ) continue;
        if( auto targ = ($eprototype)res[0]; targ ) {
            if( $(targ) and targ->dtype->is_type(StructType) ) {
                res[0] = targ->dtype->sub;
            }
        }
        if( auto cls = ($classdef)res[0]; cls ) {
            table += GetInheritTable(cls, paddings + classdefs{def});
            table << cls;
        }
    }

    return table;
}

ConvertPaths SemanticContext::Match( $typeexpr dst, $typeexpr src, typeexpres paddings ) {

    for( auto padding : paddings ) if( IsIdentical(dst,padding) ) return {};
    
    /** 检查转换可达 */
    auto converts =  CanConvert(dst, src);
    if( converts.size() == 0 or converts[0]->cost() == 0  ) converts.clear();
    else if( converts[0]->cost() == 1 ) return converts;

    /** 检查构造可达 */
    converts += CanCnostruct(dst, src, paddings);

    return converts;
}

ConvertPaths SemanticContext::CanConvert( $typeexpr dst, $typeexpr src ) {
    if( !$(dst) or !$(src) ) return {};
    if( IsIdentical(dst,src, true) ) return {new ConvertPath(1)};
    if( dst->is_type(UnknownType) ) return {new ConvertPath(2)};
    
    if( src->is_type(BasicTypeMask) ) {
        if( dst->is_type(EnumType) and src->is_type(Int32Type) )
            return {new ConvertPath(3)};
        else if( basic_type_convert_table.count({dst->id,src->id}) )
            return {new ConvertPath(basic_type_convert_table.at({dst->id,src->id}))};
    } else if( src->is_type(StructType) ) {
        ConvertPaths ret;
        for( auto def : (($classdef)src->sub)->defs )
            if( auto as = ($opdef)def; as and as->name.is(PVT::AS) and IsIdentical(as->ret_proto->dtype, dst) )
                ret << new ConvertPath(4,as);
        return ret;
    } else if( src->is_type(EntityType) ) {
        return {};
    } else if( src->is_type(CallableType) ) {
        return {};
    } else if( src->is_type(PointerTypeMask) ) {
        if( src->is_type(NullPointerType) ) {
            if( dst->is_type(Uint64Type) ) {
                return {new ConvertPath(2)};
            } else if( dst->is_type(ConstraintedPointerType) ) {
                return {new ConvertPath(2)};
            } else if( dst->is_type(UnconstraintedPointerType) ) {
                return {new ConvertPath(2)};
            } else {
                return {};
            }
        } else if( src->is_type(ConstraintedPointerType) ) {
            if( dst->is_type(Uint64Type) ) {
                return {new ConvertPath(2)};
            } else if( dst->is_type(NullPointerType) ) {
                return {new ConvertPath(2)};
            } else if( dst->is_type(ConstraintedPointerType) ) {
                return {new ConvertPath(3)};
            } else if( dst->is_type(UnconstraintedPointerType) ) {
                return {new ConvertPath(3)};
            } else {
                return {};
            }
        } else /*if( src->is_type(UnconstraintedPointerType) )*/ {
            if( dst->is_type(Uint64Type) ) {
                return {new ConvertPath(2)};
            } else if( dst->is_type(NullPointerType) ) {
                return {new ConvertPath(3)};
            } else if( dst->is_type(ConstraintedPointerType) ) {
                return {new ConvertPath(3)};
            } else if( dst->is_type(UnconstraintedPointerType) ) {
                return {new ConvertPath(3)};
            } else {
                return {};
            }
        }
    } else if( src->is_type(EnumType) ) {
        if( dst->is_type(Int32Type) )
            return {new ConvertPath(2)};
        else
            return {};
    }

    return {};
}

ConvertPaths SemanticContext::CanCnostruct( $typeexpr dst, $typeexpr src, typeexpres paddings ) {
    if( !$(dst) or !$(src) ) return {};
    if( IsIdentical(dst,src, true) ) return {new ConvertPath(1)};
    if( dst->is_type(UnknownType) ) return {new ConvertPath(2)};

    ConvertPaths converts;
    
    if( dst->is_type(StructType) ) {
        for( auto def : (($classdef)dst->sub)->defs ) {
            if( auto op = ($opdef)def; op and op->name.is(PVT::SCTOR,PVT::LCTOR) and op->arguments.size() >= 1 ) {
                $element arg;
                auto args = MinimalCall(dynamic_cast<callable*>(&*op), false);

                /** 判断自动传参 */
                if( args.size() == 1 ) arg = args[0];
                else if( op->arguments.size() == 1 ) arg = op->arguments[0];
                else continue;

                /** 判断传参类型匹配 */
                auto pres = Match(arg->proto->dtype, src, paddings + typeexpres{dst});
                if( pres.size() == 0 or pres[0]->cost() == 0 ) continue;

                /** 整理转换路径 */
                for( auto pre : pres ) 
                    converts << pre->copy()->push(new ConvertPath(5, op));
            }
        }
    }

    return converts;
}

elements SemanticContext::MinimalCall( callable* call, bool order ) {
    elements ret;
    for( auto arg : call->arguments ) {
        if( arg->init ) {
            if( order ) break;
        } else {
            ret << arg;
        }
    }
    return ret;
}

$eprototype SemanticContext::DetectElementPrototype( $exprstmt expr ) {

    if( auto e = ($constant)expr; e ) {
        $typeexpr dtype;
        eprototype::type_t etype = eprototype::var;
        bool cons = false;
        switch( e->value.id ) {
            case VT::L::CHAR : {
                dtype = typeexpr::make(e->getScope(),e->phrase, NamedType,
                    (anything)nameexpr::make(e->getScope(),e->phrase,token("char")) );
            } break;
            case VT::L::STRING : {
                dtype = typeexpr::make(e->getScope(),e->phrase, NamedType,
                    (anything)nameexpr::make(e->getScope(),e->phrase,token("string")) );
            } break;
            case VT::L::FALSE : case VT::L::TRUE : {
                dtype = typeexpr::make(e->getScope(),e->phrase, BooleanType);
            } break;
            case VT::L::FLOAT : {
                dtype = typeexpr::make(e->getScope(),e->phrase, Float64Type);
            } break;
            case VT::L::NULL : {
                dtype = typeexpr::make(e->getScope(),e->phrase, NullPointerType);
            } break;
            case VT::L::I::B : {
                const auto bits = e->value.tx.size()-2;
                const auto bytes = (bits+7)/8;
                if( bytes <= 1 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint8Type );
                else if( bytes <= 2 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint16Type );
                else if( bytes <= 4 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint32Type );
                else if( bytes <= 8 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint64Type );
                else return nullptr;
            } break;
            case VT::L::I::H : {
                const auto bits = e->value.tx.size()-2;
                const auto bytes = (bits+1)/2;
                if( bytes <= 1 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint8Type );
                else if( bytes <= 2 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint16Type );
                else if( bytes <= 4 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint32Type );
                else if( bytes <= 8 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint64Type );
                else return nullptr;
            } break;
            case VT::L::I::O : {
                const auto bits = e->value.tx.size()-2;
                const auto bytes = (bits+2)/3;
                if( bytes <= 1 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint8Type );
                else if( bytes <= 2 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint16Type );
                else if( bytes <= 4 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint32Type );
                else if( bytes <= 8 ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint64Type );
                else return nullptr;
            } break;
            case VT::L::I::N : {
                const auto value = stoull(e->value.tx);
                if( value <= INT8_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Int8Type );
                else if( value <= UINT8_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint8Type );
                else if( value <= INT16_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Int16Type );
                else if( value <= UINT16_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint16Type );
                else if( value <= INT32_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Int32Type );
                else if( value <= UINT32_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint32Type );
                else if( value <= INT64_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Int64Type );
                else if( value <= UINT64_MAX ) dtype = typeexpr::make(e->getScope(),e->phrase, Uint64Type );
                else return nullptr;
            } break;
            case VT::L::THIS : {
                auto impl = e->getScope();
                while( impl and !impl->is(node::IMPLEMENTATION) ) impl = impl->getScope();
                if( !impl ) return nullptr;
                if( auto met = ($metimpl)impl; met and met->cons ) cons = true;
                dtype = typeexpr::make(e->getScope(), e->phrase, ThisClassType);
                if( cons ) dtype = typeexpr::make(e->getScope(),e->phrase, ConstraintedPointerType, (anything)dtype );
                else dtype = typeexpr::make(e->getScope(),e->phrase, ConstraintedPointerType, (anything)dtype );
            } break;
            case VT::CLASS : {
                dtype = typeexpr::make(e->getScope(), e->phrase, ThisClassType);
            } break;
            case VT::L::LABEL: {
                dtype = typeexpr::make(e->getScope(), e->phrase, EnumType);
            } break;
            default: return nullptr;
        }
        return eprototype::make(e->getScope(), e->phrase, dtype, etype, cons?token("const"):token());
    } else if( auto e = ($nameexpr)expr; e ) {
        auto res = $(e);
        if( res.size() != 1 ) return nullptr;
        if( auto ex = ($exprstmt)res[0]; ex ) return DetectElementPrototype(ex);
        return nullptr;
    } else if( auto e = ($typeexpr)expr; e ) {
        return nullptr;
    } else if( auto e = ($monoexpr)expr; e ) {
        
    } else if( auto e = ($binexpr)expr; e ) {

    } else if( auto e = ($callexpr)expr; e ) {

    } else if( auto e = ($lambdaexpr)expr; e ) {

    } else if( auto e = ($sctorexpr)expr; e ) {

    } else if( auto e = ($lctorexpr)expr; e ) {

    } else if( auto e = ($tctorexpr)expr; e ) {

    } else if( auto e = ($newexpr)expr; e ) {

    } else if( auto e = ($delexpr)expr; e ) {

    } else if( auto e = ($doexpr)expr; e ) {

    } else if( auto e = ($tconvexpr)expr; e ) {

    } else if( auto e = ($aspectexpr)expr; e ) {

    } else if( auto e = ($mbrexpr)expr; e ) {

    }
}

SemanticContext::searching_layer::searching_layer( $scope scope, $nameexpr name ):layers(nullptr) {
    if( !scope ) return;
    auto module = scope->getModule();
    auto& context = module->sctx;
    auto& diagnostics  = context.diagnostics;
    auto found = 0;
    for( auto layer : context.searching_layers ) if( scope == layer ) {
        found += 1;
    }
    if( found >= 8) {
        diagnostics[name->getDocUri()]("111", name->name);
        return;
    }
    context.searching_layers.push(scope);
    layers = &context.searching_layers;
    return;
}

SemanticContext::searching_layer::~searching_layer() {
    if( layers ) layers->pop();
}

SemanticContext::searching_layer::operator bool()const {
    return layers;
}

}

#endif