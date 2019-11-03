#ifndef __pvt__
#define __pvt__

namespace alioth {

/**
 * @enum-class PVT : 约定
 * @desc :
 *  Alioth 0.3.1编译器将约定判定从CT中剥离出来
 */
enum class PVT {

    /** definition modifiers */
    PRIVATE, // private -
    PROTECTED, // protected *
    PUBLIC, // public +
    ABSTRACT, // abstract
    ATOMIC, // atomic
    ASYNC, // async
    RAW, // raw

    /** operator modifiers */
    PREFIX, // prefix
    SUFFIX, // suffix
    REV, // rev
    ISM, // ism

    /** operator labels */
    ADD, // add +
    SUB, // sub -
    MUL, // mul *
    DIV, // div /
    MOL, // mol %
    BITAND, // bitand &
    BITOR, // bitor |
    BITXOR, // botxor ^
    SHL, // shl <<
    SHR, // shr >>
    LT, // lt <
    GT, // gt >
    LE, // le <=
    GE, // ge >=
    EQ, // eq ==
    NE, // ne !=
    AND, // and
    OR, // or
    XOR, // xor
    NOT, // not
    INDEX, // index []
    NEGATIVE, // negative
    BITREV, // bitrev ~
    INCREMENT, // increment ++
    DECREMENT, // decrement --
    EQUAL, // assign =
    ASSIGN, // VN::OP::ASSIGN
    ASSIGN_ADD, // 'assign add' +=
    ASSIGN_SUB, // 'assign sub' -=
    ASSIGN_MUL, // 'assign mul' *=
    ASSIGN_DIV, // 'assign div' /=
    ASSIGN_MOL, // 'assign mol' %=
    ASSIGN_SHL, // 'assign shl' <<=
    ASSIGN_SHR, // 'assign shr' >>=
    ASSIGN_BITAND, // 'assign bitand' &=
    ASSIGN_BITOR, // 'assign bitor' |=
    ASSIGN_BITXOR, // 'assign bitxor' ^=
    SCTOR, // sctor {...}
    LCTOR, // lctor [...]
    DTOR, // dtor {~}
    CCTOR, // cctor
    MCTOR, // mctor
    AS, // VN::OP::AS
    POINT, // member .
    SHARP, // aspect #
    MOVE, // move
    MEMBER, // VN::OP::MEMBER
    ASPECT, // VN::OP::ASPECT

    /** statement keywords */
    IS, // is
    WHILE, // while
    FOR, // for
    ON, // on :
    THEN, // then :
    AFTER, // after
    WHEN, // when
};

}

#endif