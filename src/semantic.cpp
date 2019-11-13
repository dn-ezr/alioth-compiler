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

            string dname = GetBinarySymbol(($node)def);
            string pname = GetBinarySymbol(($node)prv);

            if( dname == pname ) {
                string str;
                for( auto [s,t] : forest ) str += (string)s->name + " ";
                diagnostics[def->getDocUri()]("79", def->name, str)
                [-1](prv->getDocUri(),"45", prv->name);
                success = false;
            }
        }
    }

    return success;
}

bool SemanticContext::validateClassDefinition(  $classdef def ) {
    return false;
}

bool SemanticContext::validateEnumDefinition(  $enumdef def ) {
    return false;
}

bool SemanticContext::validateAliasDefinition(  $aliasdef def ) {
    return false;
}

bool SemanticContext::validateAttributeDefinition(  $attrdef def ) {
    return false;
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

}

#endif