#ifndef __semantic_cpp__
#define __semantic_cpp__

#include "semantic.hpp"

namespace alioth {

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
    auto& mod = forest[sig] = new module;
    mod->sig = sig;
    for( auto [_,rec] : sig->docs ) {
        auto fg = get<1>(rec);
        mod->defs += fg->defs;
        mod->impls += fg->impls;
        fg->setScope(mod);
    }
    return true;
}

bool SemanticContext::associateModules( signatures all ) {
    bool success = true;
    for( auto sig : all )
        success = associateModule(sig) and success;
    return success;
}

bool SemanticContext::validateDefinitionSemantics() {
    return true;
}

bool SemanticContext::validateImplementationSemantics() {
    return true;
}

}

#endif