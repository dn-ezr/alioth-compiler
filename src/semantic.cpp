#ifndef __semantic_cpp__
#define __semantic_cpp__

#include "semantic.hpp"

namespace alioth {

SemanticContext::SemanticContext( CompilerContext& _cctx, Diagnostics& diag ):
    cctx(_cctx),diagnostics(diag) {

}

bool SemanticContext::associateModule( $signature sig ) {
    if( !sig ) return false;
    if( auto it = forest.find(sig); it != forest.end() and it->second ) return true;
    auto& mod = forest[sig] = new module;
    return false;
}

bool SemanticContext::associateModules( signatures ) {

}

bool SemanticContext::validateDefinitionSemantics() {

}

bool SemanticContext::validateImplementationSemantics() {

}

}

#endif