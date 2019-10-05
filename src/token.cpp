#ifndef __token_cpp__
#define __token_cpp__

#include "token.hpp"
#include "diagnostic.hpp"

using namespace std;
namespace alioth {

token::token():id(VT::R::ERR),bl(0),bc(0),el(0),ec(0) {

}

token::token(int n):id(n),bl(0),bc(0),el(0),ec(0) {
    if( VT::written_table.count(n) ) tx = VT::written_table.at(n);
}

token::token(const string& str):token(VT::L::LABEL) {
    tx = str;
}

bool token::operator == ( const token& another ) const {
    if( id != another.id ) return false;
    if( isvt(id) ) switch(id) {
        case VT::R::ERR: 
            return false;
        case VT::L::CHAR:case VT::L::LABEL:case VT::L::STRING:case VT::L::I::B:case VT::L::I::H:case VT::L::I::N:case VT::L::I::O:
            return tx == another.tx;
        default:
            return true;
    } else if( isvn(id) ) {
        return true;
    } else {
        return false;
    }
}

bool token::operator != ( const token& another ) const {
    return !(*this == another);
}

token::operator std::string()const {
    if( VT::written_table.count(id) ) return VT::written_table.at(id);
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
                VT::L::I::B,VT::L::I::H,VT::L::I::N,VT::L::I::O
            );
        case CT::ASSIGN:
            return is(
                VT::O::ASSIGN,
                VT::O::ASS::B::AND,VT::O::ASS::B::OR,VT::O::ASS::B::XOR,
                VT::O::ASS::SHL,VT::O::ASS::SHR,
                VT::O::ASS::PLUS,VT::O::ASS::MINUS,
                VT::O::ASS::MUL,VT::O::ASS::DIV,VT::O::ASS::MOL
            );
        case CT::RELATION:
            return is(
                VT::O::LT,VT::O::LE,
                VT::O::GT,VT::O::GE,
                VT::O::EQ,VT::O::NE
            );
        case CT::OPERATOR: 
            return is(
                VT::O::MEMBER,
                VT::O::SHARP,
                VT::O::INCRESS,VT::O::DECRESS,
                VT::NOT,
                VT::O::B::REV,
                VT::O::SHL,VT::O::SHR,
                VT::O::B::AND,
                VT::O::B::XOR,
                VT::O::MOL,VT::O::MUL,VT::O::DIV,
                VT::O::PLUS,VT::O::MINUS,
                VT::O::RANGE,
                CT::RELATION,
                VT::AND,
                VT::OR,
                CT::ASSIGN
            );
        case CT::PREFIX:
            return is(
                VT::O::PLUS,VT::O::MINUS,
                VT::O::B::AND,VT::O::MUL,
                VT::O::INCRESS,VT::O::DECRESS,
                VT::O::B::REV
            );
        case CT::SUFFIX:
            return is(
                VT::O::INCRESS,VT::O::DECRESS
            );
        case CT::INFIX:
            return is(
                VT::O::MEMBER,
                VT::O::SHARP,
                VT::O::SHL,VT::O::SHR,
                VT::O::B::AND,
                VT::O::B::XOR,
                VT::O::B::OR,
                VT::O::MOL,VT::O::MUL,VT::O::DIV,
                VT::O::PLUS,VT::O::MINUS,
                VT::O::RANGE,
                CT::RELATION,
                VT::AND,
                VT::OR,
                CT::ASSIGN
            );
        case CT::ELETYPE:
            return is(
                VT::OBJ,VT::PTR,VT::REF,VT::REL,VT::VAR
            );
        // case CT::MF_ABSTRACT: return tx == "abstract";
        // case CT::MF_REV: return tx == "rev";
        // case CT::MF_ISM: return tx == "ism";
        // case CT::MF_PREFIX: return tx == "prefix";
        // case CT::MF_SUFFIX: return tx == "suffix";
        // case CT::MF_ATOMIC: return tx == "atomic";
        // case CT::MF_RAW: return tx == "raw";
        // case CT::LB_SCTOR: return tx == "sctor";
        // case CT::LB_LCTOR: return tx == "lctor";
        // case CT::LB_CCTOR: return tx == "cctor";
        // case CT::LB_MCTOR: return tx == "mctor";
        // case CT::LB_DTOR: return tx == "dtor";
        // case CT::LB_MEMBER: return tx == "member" or is(VT::O::MEMBER);
        // case CT::LB_WHERE: return tx == "where" or is(VT::O::SHARP);
        // case CT::LB_MOVE: return tx == "move";
        // case CT::LB_NEGATIVE: return tx == "negative";
        // case CT::LB_BITREV: return tx == "bitrev" or is(VT::O::B::REV);
        // case CT::LB_INCREASE: return tx == "increase" or is(VT::O::INCRESS);
        // case CT::LB_DECREASE: return tx == "decrease" or is(VT::O::DECRESS);
        // case CT::LB_INDEX: return tx == "index";
        // case CT::LB_ADD: return tx == "add" or is(VT::O::PLUS);
        // case CT::LB_SUB: return tx == "sub" or is(VT::O::MINUS);
        // case CT::LB_MUL: return tx == "mul" or is(VT::O::MUL);
        // case CT::LB_DIV: return tx == "div" or is(VT::O::DIV);
        // case CT::LB_MOL: return tx == "mol" or is(VT::O::MOL);
        // case CT::LB_BITAND: return tx == "bitand" or is(VT::O::B::AND);
        // case CT::LB_BITOR: return tx == "bitor" or is(VT::O::B::OR);
        // case CT::LB_BITXOR: return tx == "bitxor" or is(VT::O::B::XOR);
        // case CT::LB_SHL: return tx == "shl" or is(VT::O::SHL);
        // case CT::LB_SHR: return tx == "shr" or is(VT::O::SHR);
        // case CT::LB_LT: return tx == "lt" or is(VT::O::LT);
        // case CT::LB_GT: return tx == "gt" or is(VT::O::GT);
        // case CT::LB_LE: return tx == "le" or is(VT::O::LE);
        // case CT::LB_GE: return tx == "ge" or is(VT::O::GE);
        // case CT::LB_EQ: return tx == "eq" or is(VT::O::EQ);
        // case CT::LB_NE: return tx == "ne" or is(VT::O::NE);
        // case CT::LB_ASSIGN: return tx == "assign" or is(VT::O::ASSIGN);
        // case CT::PP_ON: return tx == "on";
        // case CT::PP_THEN: return tx == "then";
        default:
            return false;
    }
}

bool token::is( PVT p )const {
    switch( p ) {
        case PVT::PRIVATE:
            return tx == "private" or tx == "-";
        case PVT::PROTECTED:
            return tx == "protected" or tx == "*";
        case PVT::PUBLIC:
            return tx == "public" or tx == "+";
        case PVT::ABSTRACT:
            return tx == "abstract";
        default: return false;
    }
}

tuple<bool,string,Diagnostics> token::extractContent() const {
    if( !is(VT::L::STRING,VT::L::CHAR) ) return {true,tx,{}};
    char boundary = is(VT::L::STRING)?'"':'\'';
    char another = (boundary == '"')?'\'':'"';
    Diagnostics diagnostics;
    string ret;
    bool correct = true;
    int state = 0;
    auto i = tx.begin();
    while( i != tx.end() and state >= 0 ) {
        switch( state ) {
            case 0:
                if( *i == boundary ) state = 1;
                else correct = (diagnostics("l0"), false);
                break;
            case 1:
                if( *i == boundary ) state = -1;
                else if( *i == '\\' ) switch( *++i ) {
                    case 'a': ret += '\a'; break;
                    case 'b': ret += '\b'; break;
                    case 'n': ret += '\n'; break;
                    case 'r': ret += '\r'; break;
                    case 't': ret += '\t'; break;
                    case '"': ret += '\"'; break;
                    case '\'': ret += '\''; break;
                    case '\\': ret += '\\'; break;
                    case '$': ret += '$'; break;
                    case 'x': {
                        char h = *++i;
                        char l = *++i;
                        char x = 0;
                        switch( h ) {
                            case '0' ... '9': x = h - '0'; break;
                            case 'a' ... 'f': x = 10 + h - 'a'; break;
                            case 'A' ... 'F': x = 10 + h - 'A'; break;
                            default: correct = (diagnostics("l1"), false);
                        } switch( l ) {
                            case '0' ... '9': x = x*16 + h - '0'; break;
                            case 'a' ... 'f': x = x*16 + 10 + h - 'a'; break;
                            case 'A' ... 'F': x = x*16 + 10 + h - 'A'; break;
                            default: correct = (diagnostics("l1"), false);
                        }
                        ret += x;
                    } break;
                    default: correct = (diagnostics("l1"), false);
                } else {
                    ret += *i;
                } break;
        }
        i++;
    }

    return {correct,ret,diagnostics};
}

bool token::islabel() const {
    return islabel(tx);
}

bool token::islabel( const string& str ) {
    if( str.empty() ) return false;
    if( !isalpha(str[0]) and str[0] != '_' ) return false;
    for( int i = 1; i < str.size(); i++ )
        if( !isalnum(str[i]) and str[i] != '_' ) return false;
    return true;
}

}
#endif