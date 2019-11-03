#ifndef __vt__
#define __vt__

#include <map>

#undef NULL
namespace alioth {
using namespace std;
/**
 * @namespace VT: 终结符词汇表
 * @desc :
 *  终结符词汇表将所有终结符使用数字加以区别
 *  利用行号区别各个值
 */
#define DEFINE(x) constexpr int x = __LINE__ + offset;
namespace VT { // vocabulary-terminal
    constexpr int offset = 100;
    
    DEFINE(min)
    namespace R { // reserved token
        constexpr int ERR = 0; // 无效的记号，token的默认值
        constexpr int BEG = 1; // 源文档的起始记号
        constexpr int END = -1; // 源文档的结束记号
    }
    namespace O { // operator
        DEFINE(ASSIGN) // =
        namespace ASS { // assign operator
            DEFINE(DIV) // /=
            DEFINE(MINUS) // -=
            DEFINE(MOL) // %=
            DEFINE(MUL) // *=
            DEFINE(PLUS) // +=
            DEFINE(SHL) // <<=
            DEFINE(SHR) // >>=
            namespace B { // binary assign operator
                DEFINE(AND) // &=
                DEFINE(OR) // |=
                DEFINE(XOR) // ^=
            }
        }
        namespace B { // binary operator
            DEFINE(AND) // &
            DEFINE(OR) // |
            DEFINE(REV) // ~
            DEFINE(XOR) // ^
        }
        namespace SC { // scoping operator
            namespace C { // closing scope
                DEFINE(A) // )
                DEFINE(L) // ]
                DEFINE(S) // }
            }
            namespace O { // opening scope
                DEFINE(A) // (
                DEFINE(L) // [
                DEFINE(S) // {
            }
            DEFINE(COLON) // :
            DEFINE(COMMA) // ,
            DEFINE(SEMI) // ;
        }
        DEFINE(AND) // and
        DEFINE(AS) // as
        DEFINE(ASK) // ?
        DEFINE(AT) // @
        DEFINE(CONV) // $
        DEFINE(DECREMENT) // --
        DEFINE(DIV) // /
        DEFINE(EQ) // ==
        DEFINE(ETC) // ...
        DEFINE(EXCEPTION) // !!
        DEFINE(FORCE) // !
        DEFINE(GE) // >=
        DEFINE(GENERATE) // =>
        DEFINE(GT) // >
        DEFINE(INCREMENT) // ++
        DEFINE(LE) // <=
        DEFINE(LT) // <
        DEFINE(MEMBER) // .
        DEFINE(MINUS) // -
        DEFINE(MOL) // %
        DEFINE(MUL) // *
        DEFINE(NE) // !=
        DEFINE(NOT) // not
        DEFINE(OR) // or
        DEFINE(PLUS) // +
        DEFINE(POINTER) // ->
        DEFINE(RANGE) // ..
        DEFINE(SCOPE) // ::
        DEFINE(SHARP) // #
        DEFINE(SHL) // <<
        DEFINE(SHR) // >>
        DEFINE(TREAT) // as!
        DEFINE(XOR) // xor
    }
    namespace U {
        DEFINE(SPACE)
        namespace C { // comment
            DEFINE(LINE)
            DEFINE(BLOCK)
        }
    }
    namespace L { // literal
        DEFINE(CHAR)
        DEFINE(FALSE)
        DEFINE(FLOAT)
        namespace I { // literal integer
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
    DEFINE(LET)
    DEFINE(LOOP)
    DEFINE(META)
    DEFINE(METHOD)
    DEFINE(MODULE)
    DEFINE(NEW)
    DEFINE(OBJ)
    DEFINE(OPERATOR)
    DEFINE(OTHERWISE)
    DEFINE(PTR)
    DEFINE(REF)
    DEFINE(REL)
    DEFINE(RETURN)
    DEFINE(SWITCH)
    DEFINE(UINT16)
    DEFINE(UINT32)
    DEFINE(UINT64)
    DEFINE(UINT8)
    DEFINE(VAR)
    DEFINE(VOID)

    /** @max : 用于方便随时确定终结符的最大边界 */
    DEFINE(max)

    /** @written_table : 终结词汇书写表 */
    extern const map<int,string> written_table;
}

}

#endif