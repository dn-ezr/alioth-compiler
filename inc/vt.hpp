#ifndef __vt__
#define __vt__
#undef NULL
namespace alioth {
/**
 * @namespace VT: 终结符词汇表
 * @desc :
 *  终结符词汇表将所有终结符使用数字加以区别
 *  利用行号区别各个值
 */
#define DEFINE(x) const static int x = __LINE__ + offset;
namespace VT {
    const static int offset = 100;
    
    namespace R {
        const static int ERR = 0;
        const static int BEG = 1;
        const static int END = -1;
    }
    namespace O {
        namespace ASS {
            DEFINE($)
            DEFINE(DIV)
            DEFINE(MINUS)
            DEFINE(MOL)
            DEFINE(MUL)
            DEFINE(PLUS)
            DEFINE(SHL)
            DEFINE(SHR)
            namespace B {
                DEFINE(AND)
                DEFINE(OR)
                DEFINE(XOR)
            }
        }
        namespace B {
            DEFINE(AND)
            DEFINE(OR)
            DEFINE(REV)
            DEFINE(XOR)
        }
        namespace SC {
            namespace C {
                DEFINE(A)
                DEFINE(L)
                DEFINE(S)
            }
            namespace O {
                DEFINE(A)
                DEFINE(L)
                DEFINE(S)
            }
            DEFINE(COLON)
            DEFINE(COMMA)
            DEFINE(SEMI)
        }
        DEFINE(ASK)
        DEFINE(AT)
        DEFINE(CONV)
        DEFINE(DECRESS)
        DEFINE(DIV)
        DEFINE(EQ)
        DEFINE(ETC)
        DEFINE(EXCEPTION)
        DEFINE(FORCE)
        DEFINE(GE)
        DEFINE(GT)
        DEFINE(INCRESS)
        DEFINE(LE)
        DEFINE(LT)
        DEFINE(MINUS)
        DEFINE(MOL)
        DEFINE(MUL)
        DEFINE(NE)
        DEFINE(PLUS)
        DEFINE(RANGE)
        DEFINE(SCOPE)
        DEFINE(SHARP)
        DEFINE(SHL)
        DEFINE(SHR)
        DEFINE(XOR)
    }
    namespace U {
        DEFINE(SPACE)
    }
    namespace COMMENT {
        DEFINE(LINE)
        DEFINE(BLOCK)
    }
    namespace L {
        DEFINE(CHAR)
        DEFINE(FALSE)
        DEFINE(FLOAT)
        namespace I {
            DEFINE(B)
            DEFINE(H)
            DEFINE(N)
            DEFINE(O)
        }
        DEFINE(LABEL)
        DEFINE(NULL)
        DEFINE(STRING)
        DEFINE(THIS)
        DEFINE(TRUE)
    }
    DEFINE(AND)
    DEFINE(AS)
    DEFINE(ASM)
    DEFINE(ASSUME)
    DEFINE(BOOL)
    DEFINE(BREAK)
    DEFINE(CASE)
    DEFINE(CLASS)
    DEFINE(CONST)
    DEFINE(CONTINUE)
    DEFINE(DEFAULT)
    DEFINE(DELETE)
    DEFINE(DO)
    DEFINE(ELSE)
    DEFINE(ENTRY)
    DEFINE(ENUM)
    DEFINE(FLOAT32)
    DEFINE(FLOAT64)
    DEFINE(IF)
    DEFINE(INT16)
    DEFINE(INT32)
    DEFINE(INT64)
    DEFINE(INT8)
    DEFINE(LOOP)
    DEFINE(MEMBER)
    DEFINE(META)
    DEFINE(METHOD)
    DEFINE(MODULE)
    DEFINE(NEW)
    DEFINE(NOT)
    DEFINE(OBJ)
    DEFINE(OPERATOR)
    DEFINE(OR)
    DEFINE(OTHERWISE)
    DEFINE(PTR)
    DEFINE(REF)
    DEFINE(REL)
    DEFINE(RETURN)
    DEFINE(SWITCH)
    DEFINE(TREAT)
    DEFINE(UINT16)
    DEFINE(UINT32)
    DEFINE(UINT64)
    DEFINE(UINT8)
    DEFINE(VOID)

    /** @max : 用于方便随时确定终结符的最大边界 */
    DEFINE(max)
}

}

#endif