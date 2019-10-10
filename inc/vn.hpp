#ifndef __vn__
#define __vn__

#include "vt.hpp"

namespace alioth {
/**
 * @enum-class VN : 非终结符ID
 * @desc : 用于区分不同非终结符的ID
 */
namespace VN {
    constexpr int offset = VT::max + 1000;
    DEFINE(min)

    DEFINE(MODULE)
    DEFINE(DEPENDENCY)

    DEFINE(ELEPROTO)

    DEFINE(ALIASDEF)
    DEFINE(CLASSDEF)
    DEFINE(ENUMDEF)
    DEFINE(OPDEF)
    DEFINE(METDEF)
    DEFINE(ATTRDEF)
    
    DEFINE(NAMEEXPR)
    DEFINE(TYPEEXPR)

    DEFINE(LIST) // 临时非终结符
    DEFINE(ITEM) // 临时非终结符
    DEFINE(max)
};

}

#endif