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

    DEFINE(METIMPL)
    DEFINE(OPIMPL)
    
    DEFINE(BLOCKSTMT)
    DEFINE(ELEMENTSTMT)
    DEFINE(FCTRLSTMT)
    DEFINE(EXPRSTMT)
    DEFINE(BRANCHSTMT)
    DEFINE(SWITCHSTMT)
    DEFINE(LOOPSTMT)
    DEFINE(ASSUMESTMT)
    DEFINE(DOSTMT)
    DEFINE(TRYSTMT)
    DEFINE(CATCHSTMT)
    DEFINE(THROWSTMT)
    
    DEFINE(NAMEEXPR)
    DEFINE(TYPEEXPR)

    namespace OP {
        DEFINE(INDEX)
        DEFINE(SCTOR)
        DEFINE(LCTOR)
        DEFINE(DTOR)
        DEFINE(CCTOR)
        DEFINE(MCTOR)
        DEFINE(MEMBER)
        DEFINE(AS)
        DEFINE(ASPECT)
        DEFINE(ASSIGN)
        namespace ASS {
            DEFINE(ADD)
            DEFINE(SUB)
            DEFINE(MUL)
            DEFINE(DIV)
            DEFINE(MOL)
            DEFINE(SHL)
            DEFINE(SHR)
            DEFINE(BITAND)
            DEFINE(BITOR)
            DEFINE(BITXOR)
        }
    }

    DEFINE(LIST) // 临时非终结符
    DEFINE(ITEM) // 临时非终结符
    DEFINE(FINAL)// 临时非终结符
    DEFINE(MORE) // 临时非终结符
    DEFINE(max)
};

}

#endif