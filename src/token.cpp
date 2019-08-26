#ifndef __token_cpp__
#define __token_cpp__

#include "token.hpp"

using namespace std;
namespace alioth {

token::token():id(VT::R::ERR),bl(0),bc(0),el(0),ec(0) {

}

token::token(int n):id(n),bl(0),bc(0),el(0),ec(0) {

}

token::token(const string& str):token(VT::L::LABEL) {
    tx = str;
}

token::operator std::string()const {
    // return Xengine::written(*this);
    return tx;
}

token::operator bool()const {
    return !is(VT::R::ERR);
}

bool token::is( int v )const {
    return id == v;
}

bool token::is( CT v )const {
    switch( v ) {
        case CT::BASIC_TYPE:
            return is(
                VT::INT8,VT::INT16,VT::INT32,VT::INT64,
                VT::UINT8,VT::UINT16,VT::UINT32,VT::UINT64,
                VT::FLOAT32,VT::FLOAT64,
                VT::BOOL,
                VT::VOID);
        case CT::CONSTANT:
            return is(
                VT::L::CHAR,VT::L::STRING,
                VT::L::FALSE,VT::L::TRUE,
                VT::L::NULL, //VT::VOID, [2019/03/28] 将nil从常量中剔除,从此引用不存在空值
                VT::L::FLOAT,
                VT::L::INTEGERb,VT::L::INTEGERh,VT::L::INTEGERn,VT::L::INTEGERo
            );
        case CT::ASSIGN:
            return is(
                VT::A::ASSIGN,
                VT::A::B::AND,VT::A::B::OR,VT::A::B::XOR,
                VT::A::SHL,VT::A::SHR,
                VT::A::PLUS,VT::A::MINUS,
                VT::A::MUL,VT::A::DIV,VT::A::MOL
            );
        case CT::RELATION:
            return is(
                VT::LT,VT::LE,
                VT::GT,VT::GE,
                VT::EQ,VT::NE
            );
        case CT::OPERATOR: 
            return is(
                VT::MEMBER,
                VT::WHERE,
                VT::INCRESS,VT::DECRESS,
                VT::NOT,
                VT::B::REV,
                VT::SHL,VT::SHR,
                VT::B::AND,
                VT::B::XOR,
                VT::MOL,VT::MUL,VT::DIV,
                VT::PLUS,VT::MINUS,
                VT::RANGE,
                CT::RELATION,
                VT::AND,
                VT::OR,
                CT::ASSIGN
            );
        case CT::PREFIX:
            return is(
                VT::PLUS,VT::MINUS,
                VT::B::AND,VT::MUL,
                VT::INCRESS,VT::DECRESS,
                VT::B::REV
            );
        case CT::SUFFIX:
            return is(
                VT::INCRESS,VT::DECRESS
            );
        case CT::INFIX:
            return is(
                VT::MEMBER,
                VT::WHERE,
                VT::SHL,VT::SHR,
                VT::B::AND,
                VT::B::XOR,
                VT::B::OR,
                VT::MOL,VT::MUL,VT::DIV,
                VT::PLUS,VT::MINUS,
                VT::RANGE,
                CT::RELATION,
                VT::AND,
                VT::OR,
                CT::ASSIGN
            );
        case CT::ELETYPE:
            return is(
                VT::OBJ,VT::PTR,VT::REF,VT::REL
            );
        case CT::IMPLEMENTATION:
            return is(
                VN::BRANCH,VN::LOOP,VN::CONTROL,VN::BLOCK,VN::EXPRESSION
            );
        case CT::MF_ABSTRACT: return tx == "abstract";
        case CT::MF_REV: return tx == "rev";
        case CT::MF_ISM: return tx == "ism";
        case CT::MF_PREFIX: return tx == "prefix";
        case CT::MF_SUFFIX: return tx == "suffix";
        case CT::MF_ATOMIC: return tx == "atomic";
        case CT::MF_RAW: return tx == "raw";
        case CT::LB_SCTOR: return tx == "sctor";
        case CT::LB_LCTOR: return tx == "lctor";
        case CT::LB_CCTOR: return tx == "cctor";
        case CT::LB_MCTOR: return tx == "mctor";
        case CT::LB_DTOR: return tx == "dtor";
        case CT::LB_MEMBER: return tx == "member" or is(VT::MEMBER);
        case CT::LB_WHERE: return tx == "where" or is(VT::WHERE);
        case CT::LB_MOVE: return tx == "move";
        case CT::LB_NEGATIVE: return tx == "negative";
        case CT::LB_BITREV: return tx == "bitrev" or is(VT::B::REV);
        case CT::LB_INCREASE: return tx == "increase" or is(VT::INCRESS);
        case CT::LB_DECREASE: return tx == "decrease" or is(VT::DECRESS);
        case CT::LB_INDEX: return tx == "index";
        case CT::LB_ADD: return tx == "add" or is(VT::PLUS);
        case CT::LB_SUB: return tx == "sub" or is(VT::MINUS);
        case CT::LB_MUL: return tx == "mul" or is(VT::MUL);
        case CT::LB_DIV: return tx == "div" or is(VT::DIV);
        case CT::LB_MOL: return tx == "mol" or is(VT::MOL);
        case CT::LB_BITAND: return tx == "bitand" or is(VT::B::AND);
        case CT::LB_BITOR: return tx == "bitor" or is(VT::B::OR);
        case CT::LB_BITXOR: return tx == "bitxor" or is(VT::B::XOR);
        case CT::LB_SHL: return tx == "shl" or is(VT::SHL);
        case CT::LB_SHR: return tx == "shr" or is(VT::SHR);
        case CT::LB_LT: return tx == "lt" or is(VT::LT);
        case CT::LB_GT: return tx == "gt" or is(VT::GT);
        case CT::LB_LE: return tx == "le" or is(VT::LE);
        case CT::LB_GE: return tx == "ge" or is(VT::GE);
        case CT::LB_EQ: return tx == "eq" or is(VT::EQ);
        case CT::LB_NE: return tx == "ne" or is(VT::NE);
        case CT::LB_ASSIGN: return tx == "assign" or is(VT::A::ASSIGN);
        case CT::OPL:
            return is(
                VN::O::INDEX,CT::OPL_ASSIGN,
                VN::O::SCTOR, VN::O::LCTOR, VN::O::CCTOR, VN::O::MCTOR, VN::O::DTOR, VN::O::MEMBER, VN::O::WHERE, VN::O::MOVE,VN::O::AS,
                VN::O::NEGATIVE, VN::O::B::REV, VN::O::INCREASE, VN::O::DECREASE,VN::O::NOT,
                VN::O::ADD, VN::O::SUB, VN::O::MUL, VN::O::DIV, VN::O::MOL,
                VN::O::B::AND, VN::O::B::OR, VN::O::B::XOR, VN::O::SHL, VN::O::SHR,
                VN::O::LT, VN::O::GT, VN::O::LE, VN::O::GE, VN::O::EQ, VN::O::NE,
                VN::O::AND,VN::O::OR,VN::O::XOR
            );
        case CT::OPL_ASSIGN:
            return is(
                VN::O::A::ASSIGN,
                VN::O::A::ADD, VN::O::A::SUB, VN::O::A::MUL, VN::O::A::DIV, VN::O::A::MOL,
                VN::O::A::SHL, VN::O::A::SHR, VN::O::A::B::AND, VN::O::A::B::OR, VN::O::A::B::XOR
            );
        case CT::OPL_SPECIAL:
            return is(
                VN::O::SCTOR, VN::O::LCTOR, VN::O::CCTOR, VN::O::MCTOR, VN::O::DTOR, VN::O::WHERE, VN::O::MOVE, VN::O::MEMBER, VN::O::AS
            );
        case CT::OPL_MONO:
            return is(
                VN::O::NEGATIVE, VN::O::B::REV, VN::O::INCREASE, VN::O::DECREASE, VN::O::NOT
            );
        case CT::OPL_BINO:
            return is(
                CT::OPL_ASSIGN,
                VN::O::ADD,VN::O::SUB,VN::O::MUL,VN::O::DIV,VN::O::MOL,
                VN::O::B::AND,VN::O::B::OR,VN::O::B::XOR,VN::O::SHL,VN::O::SHR,
                VN::O::LT,VN::O::GT,VN::O::LE,VN::O::GE,VN::O::EQ,VN::O::NE,
                VN::O::AND,VN::O::OR,VN::O::XOR
            );
        case CT::PP_ON: return tx == "on";
        case CT::PP_THEN: return tx == "then";
        default:
            return false;
    }
}

}
#endif