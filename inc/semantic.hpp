#ifndef __semantic__
#define __semantic__

#include "syntax.hpp"

namespace alioth {

struct module : public signature {
    public:
        definitions defs;
        implementations impls;
        fragments frgmnts;
};

}

#endif