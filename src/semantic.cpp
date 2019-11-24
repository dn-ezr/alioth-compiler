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
            diagnostics[def->getDocUri()]("79", def->name, dname)
            [-1](prv->getDocUri(),"45", prv->name);
            success = false;
        }
    }

    return success;
}

bool SemanticContext::validateClassDefinition(  $classdef cls ) {
    bool success = true;

    /** 检查基类是否可达 */
    for( auto super : cls->supers ) {
        auto res = Reach(super, SearchOption::ALL|SearchOption::ANY);
        if( res.size() != 1 ) {
            diagnostics[cls->getDocUri()]("82", super->phrase);
            success = false;
        }
        auto def = ($classdef)res[0];
        if( !def or !CanBeInstanced(def) ) {
            diagnostics[cls->getDocUri()]("83", super->phrase);
            success = false;
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
            diagnostics[def->getDocUri()]("79", def->name, dname)
            [-1](prv->getDocUri(),"45", prv->name);
            success = false;
        }

        /** 检查谓词 */
    }

    /** 检查循环包含 */

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

    auto result = Reach(def->tagret, SearchOption::ALL|SearchOption::ANY);

    success = result.size();

    return success;
}

bool SemanticContext::validateAttributeDefinition(  $attrdef def ) {
    
    bool success = true;

    return success;
}

bool SemanticContext::validateMethodDefinition(  $metdef def ) {
    return false;
}

bool SemanticContext::validateOperatorDefinition(  $opdef def ) {
    return false;
}

bool SemanticContext::validateMethodImplementation( $metimpl impl ) {
    return false;
}

bool SemanticContext::validateOperatorImplementation( $opimpl impl ) {
    return false;
}


string SemanticContext::GetBinarySymbol( $node s ) {
    if( !s ) return "<error>";
    string symbol;
    auto mod = s->getModule();
    auto src = s;

    if( mod ) {
        if( mod->sctx.symbol_cache.count(s) )
            return mod->sctx.symbol_cache[s];
    }

    if( auto impl = ($implementation)s; impl ) {
        auto def = GetDefinition(impl);
        if( !def ) return "<error>";
        else src = def;
    }

    if( auto def = ($definition)src; def ) {
        symbol = def->name;
        for( auto d = def->getScope(); d != nullptr; d = d->getScope() ) {
            if( auto cd = ($classdef)d; cd ) symbol = (string)cd->name + "::" + symbol;
            else if( auto md = ($module)d; md ) symbol = (string)md->sig->name + "::" + symbol;
        }
        if( auto cdef = ($classdef)def; cdef ) symbol = "class:" + symbol;
        else if( auto edef = ($enumdef)def; edef ) symbol = "enum:" + symbol;
        else if( auto mdef = ($metdef)def; mdef ) symbol = "method:" + symbol;
        else if( auto odef = ($opdef)def; odef ) symbol = "operator:" + symbol;
        else if( auto adef = ($attrdef)def; adef ) symbol = "attribute:" + symbol;
        else if( auto idef = ($aliasdef)def; idef ) symbol += "alias:" + symbol;
        else return "<error>";

        if( auto call = dynamic_cast<callable*>(&*def); call ) {
            symbol += "(";
            for( int i = 0; i < call->arg_protos.size(); i++ ) {
                if( i != 0 ) symbol += ",";
                symbol += GetBinarySymbol(($node)call->arg_protos[i]);
            }
            symbol += ")";
            symbol += GetBinarySymbol(($node)call->ret_proto);
        }
    } else if( auto proto = ($eprototype)src; proto ) {

    } else if( auto type = ($typeexpr)src; type ) {

    } else if( auto stmt = ($statement)src; stmt ) {
        symbol = stmt->name;
    }

    if( mod ) mod->sctx.symbol_cache[s] = symbol;
    return symbol;
}

$definition SemanticContext::GetDefinition( $implementation impl ) {
    return nullptr;
}

everything SemanticContext::Reach( $nameexpr name, SearchOptions opts, $scope scope, aliasdefs paddings ) {
    if( !name ) return {};
    if( !scope ) scope = name->getScope();
    while( scope and (!scope->isscope() or scope->is(node::FRAGMENT)) ) scope = scope->getScope();
    if( !scope or !scope->isscope() ) return {};

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
            } else { //[TODO] 考虑定义前提
                if( (opts & SearchOption::INNERS) == 0 ) continue;
                if( def->name == name->name ) results << (anything)def;
            }
        }
    } else if( auto sc = ($implementation)scope; sc ) {
        for( auto arg : sc->args ) {
            if( arg->name == name->name )
                results << (anything)arg;
        }
    } else if( auto sc = ($blockstmt)scope; sc ) {
        for( auto stmt : *sc ) {
            if( stmt->name == name->name )
                results << (anything)stmt;
        }
    } else if( auto sc = ($loopstmt)scope; sc ) {
        if( sc->it->name == name->name )
            results << (anything)sc->it;
    } else if( auto sc = ($assumestmt)scope; sc ) {
        if( sc->variable->name == name->name )
            results << (anything)sc->variable;
    } else if( auto sc = ($lambdaexpr)scope; sc ) {
        for( auto arg : sc->args ) {
            if( arg->name == name->name )
                results << (anything)arg;
        }
    } else {
        return internal_error, nothing;
    }

    if( results.size() != 0 and name->next ) {
        /** 处理作用域深入的情况 */
        if( results.size() != 1 )
            return diagnostics[name->getDocUri()]("88", name->name), nothing;
        /** 处理目标并非作用域的情况 */
        auto sc = ($node)results[0];
        if( !sc or sc->isscope() )
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
        } else if( opts & SearchOption::PARENT ) {
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
            return {};
        }

        /** 解析别名引用 */
        auto res = Reach(alias->tagret, 
            SearchOption::ALL|SearchOption::ANY, 
            alias->getScope(), 
            paddings+aliasdefs{alias} );
        if( res.size() == 0 ) return {};
        results += res;
        results.remove(i--);
    }

    return results;
}

bool SemanticContext::CanBeInstanced( $classdef def ) {
    return true;//[TODO]
}

}

#endif