#ifndef __vn__
#define __vn__

#include "vt.hpp"

namespace alioth {
/**
 * @enum-class VN : 非终结符ID
 * @desc : 用于区分不同非终结符的ID
 */
namespace VN {
    const static int offset = VT::max + 1000;
    DEFINE(ATTRIBUTE)
    DEFINE(BLOCK)
    DEFINE(BRANCH)
    namespace C {
        DEFINE(BASE_CLASSES)
        DEFINE(PREDICATE)
        DEFINE(TEMPLATE_PARAMLIST)
    }
    DEFINE(CLASS)
    DEFINE(COMMA_PARAM)
    DEFINE(CONST)
    DEFINE(CONSTRUCTOR)
    DEFINE(CONTROL)
    DEFINE(DEPENDENCY)
    DEFINE(ELEMENT)
    DEFINE(ENUM)
    DEFINE(EXPRESSION)
    DEFINE(FINAL_PARAM)
    DEFINE(LIST)
    DEFINE(LOOP)
    DEFINE(METHOD)
    DEFINE(MODULE)
    DEFINE(NAMEUC_ATOM)
    DEFINE(NAMEUC)
    DEFINE(OPERATOR)
    namespace O {
        DEFINE(ADD)
        DEFINE(AND)
        DEFINE(AS)
        namespace A {
            DEFINE(ADD)
            namespace B {
                DEFINE(AND)
                DEFINE(OR)
                DEFINE(XOR)
            }
            DEFINE(DIV)
            DEFINE(MOL)
            DEFINE(MUL)
            DEFINE(SHL)
            DEFINE(SHR)
            DEFINE(SUB)
            DEFINE(ASSIGN)
        }
        namespace B {
            DEFINE(AND)
            DEFINE(OR)
            DEFINE(REV)
            DEFINE(XOR)
        }
        DEFINE(CCTOR)
        DEFINE(DECREASE)
        DEFINE(DIV)
        DEFINE(DTOR)
        DEFINE(EQ)
        DEFINE(GE)
        DEFINE(GT)
        DEFINE(INCREASE)
        DEFINE(INDEX)
        DEFINE(LCTOR)
        DEFINE(LE)
        DEFINE(LT)
        DEFINE(MCTOR)
        DEFINE(MEMBER)
        DEFINE(MOL)
        DEFINE(MOVE)
        DEFINE(MUL)
        DEFINE(NE)
        DEFINE(NEGATIVE)
        DEFINE(NOT)
        DEFINE(OR)
        DEFINE(SCTOR)
        DEFINE(SHL)
        DEFINE(SHR)
        DEFINE(SUB)
        DEFINE(WHERE)
        DEFINE(XOR)
    }
    DEFINE(PARAM_LIST)
    DEFINE(PARAM)
    DEFINE(PROTO)
    DEFINE(RAW)
    DEFINE(TERMINAL)
    DEFINE(TYPEUC)
};

}

#endif