#ifndef __semantic__
#define __semantic__

#include "syntax.hpp"
#include "context.hpp"
#include "air.hpp"

namespace alioth {

struct module : public signature {
    public:
        definitions defs;
        implementations impls;
        fragments frgmnts;
};

class SemanticContext {

    public:
        SemanticContext( CompilerContext& cctx );
        bool associateModule( $signature );
        bool associateModules( signatures );
        bool validateDefinitionSemantics();
        bool validateImplementationSemantics();
        bool attachAirContext( air::Context& air );
        bool generateAirModule( $signature );
        bool generateAirModules( signatures );
};

}

#endif