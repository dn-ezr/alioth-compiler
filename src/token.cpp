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
    return (string)*this == (string)another;
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
                VT::O::MEMBER,VT::O::POINTER,VT::O::SHARP,
                VT::O::INCREMENT,VT::O::DECREMENT,
                VT::O::NOT,
                VT::O::B::REV,
                VT::O::SHL,VT::O::SHR,
                VT::O::B::AND,
                VT::O::B::XOR,
                VT::O::MOL,VT::O::MUL,VT::O::DIV,
                VT::O::PLUS,VT::O::MINUS,
                VT::O::RANGE,
                CT::RELATION,
                VT::O::AND,
                VT::O::OR,
                CT::ASSIGN
            );
        case CT::PREFIX:
            return is(
                VT::O::MINUS,
                VT::O::B::AND,VT::O::MUL,
                VT::O::INCREMENT,VT::O::DECREMENT,
                VT::O::B::REV,
                VT::O::NOT
            );
        case CT::SUFFIX:
            return is(
                VT::O::INCREMENT,VT::O::DECREMENT
            );
        case CT::INFIX:
            return is(
                VT::O::SHL,VT::O::SHR,
                VT::O::B::AND,
                VT::O::B::XOR,
                VT::O::B::OR,
                VT::O::MOL,VT::O::MUL,VT::O::DIV,
                VT::O::PLUS,VT::O::MINUS,
                CT::RELATION,
                VT::O::AND,
                VT::O::OR,
                CT::ASSIGN
            );
        case CT::ELETYPE:
            return is(
                VT::OBJ,VT::PTR,VT::REF,VT::REL
            );
        case CT::OP_LABEL:
            return is(
                PVT::ADD,
                PVT::SUB,
                PVT::MUL,
                PVT::DIV,
                PVT::MOL,
                PVT::BITAND,
                PVT::BITOR,
                PVT::BITXOR,
                PVT::SHL,
                PVT::SHR,
                PVT::LT,
                PVT::GT,
                PVT::LE,
                PVT::GE,
                PVT::EQ,
                PVT::NE,
                PVT::AND,
                PVT::OR,
                PVT::XOR,
                PVT::NOT,
                PVT::INDEX,
                PVT::NEGATIVE,
                PVT::BITREV,
                PVT::INCREMENT,
                PVT::DECREMENT,
                PVT::ASSIGN,
                PVT::ASSIGN_ADD,
                PVT::ASSIGN_SUB,
                PVT::ASSIGN_MUL,
                PVT::ASSIGN_DIV,
                PVT::ASSIGN_MOL,
                PVT::ASSIGN_SHL,
                PVT::ASSIGN_SHR,
                PVT::ASSIGN_BITAND,
                PVT::ASSIGN_BITOR,
                PVT::ASSIGN_BITXOR,
                PVT::SCTOR,
                PVT::LCTOR,
                PVT::DTOR,
                PVT::CCTOR,
                PVT::MCTOR,
                PVT::AS,
                PVT::MOVE,
                PVT::MEMBER,
                PVT::ASPECT
            );
        case CT::IMPLEMENTATION:
            return is(
                VN::OPIMPL,
                VN::METIMPL
            );
        case CT::STATEMENT:
            return is(
                VN::BLOCKSTMT,
                VN::ELEMENTSTMT,
                VN::FCTRLSTMT,
                VN::EXPRSTMT,
                VN::BRANCHSTMT,
                VN::SWITCHSTMT,
                VN::LOOPSTMT,
                VN::ASSUMESTMT,
                VN::DOSTMT,
                VN::TRYSTMT,
                VN::CATCHSTMT,
                VN::THROWSTMT
            );
        case CT::TERMINATOR:
            return is(
                VT::O::SC::C::A,
                VT::O::SC::C::L,
                VT::O::SC::C::S,
                VT::O::SC::SEMI,
                VT::O::SC::COMMA,
                VT::O::SC::COLON
            );
        default:
            return false;
    }
}

bool token::is( PVT p )const {
    switch( p ) {
        case PVT::PRIVATE: return tx == "private" or tx == "-";
        case PVT::PROTECTED: return tx == "protected" or tx == "*";
        case PVT::PUBLIC: return tx == "public" or tx == "+";
        case PVT::ABSTRACT: return tx == "abstract";
        case PVT::ATOMIC: return tx == "atomic";
        case PVT::ASYNC: return tx == "async";
        case PVT::RAW:  return tx == "raw";

        case PVT::PREFIX: return tx == "prefix";
        case PVT::SUFFIX: return tx == "suffix";
        case PVT::REV: return tx == "rev";
        case PVT::ISM: return tx == "ism";

        case PVT::ADD: return tx == "add" or is(VT::O::PLUS);
        case PVT::SUB: return tx == "sub" or is(VT::O::MINUS);
        case PVT::MUL: return tx == "mul" or is(VT::O::MUL);
        case PVT::DIV: return tx == "div" or is(VT::O::DIV);
        case PVT::MOL: return tx == "mol" or is(VT::O::MOL);
        case PVT::BITAND: return tx == "bitand" or is(VT::O::B::AND);
        case PVT::BITOR: return tx == "bitor" or is(VT::O::B::OR);
        case PVT::BITXOR: return tx == "bitxor" or is(VT::O::B::XOR);
        case PVT::SHL: return tx == "shl" or is(VT::O::SHL);
        case PVT::SHR: return tx == "shr" or is(VT::O::SHR);
        case PVT::LT: return tx == "lt" or is(VT::O::LT);
        case PVT::GT: return tx == "gt" or is(VT::O::GT);
        case PVT::LE: return tx == "le" or is(VT::O::LE);
        case PVT::GE: return tx == "ge" or is(VT::O::GE);
        case PVT::EQ: return tx == "eq" or is(VT::O::EQ);
        case PVT::NE: return tx == "ne" or is(VT::O::NE);
        case PVT::AND: return is(VT::O::AND);
        case PVT::OR: return is(VT::O::OR);
        case PVT::XOR: return is(VT::O::XOR);
        case PVT::NOT: return is(VT::O::NOT);
        case PVT::INDEX: return tx == "index" or is(VN::OP::INDEX);
        case PVT::NEGATIVE: return tx == "negative";
        case PVT::BITREV: return tx == "bitrev" or is(VT::O::B::REV);
        case PVT::INCREMENT: return tx == "increment" or is(VT::O::INCREMENT);
        case PVT::DECREMENT: return tx == "decrement" or is(VT::O::DECREMENT);
        case PVT::EQUAL: return tx == "assign" or is(VT::O::ASSIGN);
        case PVT::ASSIGN_ADD: return is(VT::O::ASS::PLUS) or is(VN::OP::ASS::ADD);
        case PVT::ASSIGN_SUB: return is(VT::O::ASS::MINUS) or is(VN::OP::ASS::SUB);
        case PVT::ASSIGN_MUL: return is(VT::O::ASS::MUL) or is(VN::OP::ASS::MUL);
        case PVT::ASSIGN_DIV: return is(VT::O::ASS::DIV) or is(VN::OP::ASS::DIV);
        case PVT::ASSIGN_MOL: return is(VT::O::ASS::MOL) or is(VN::OP::ASS::MOL);
        case PVT::ASSIGN_SHL: return is(VT::O::ASS::SHL) or is(VN::OP::ASS::SHL);
        case PVT::ASSIGN_SHR: return is(VT::O::ASS::SHR) or is(VN::OP::ASS::SHR);
        case PVT::ASSIGN_BITAND: return is(VT::O::ASS::B::AND) or is(VN::OP::ASS::BITAND);
        case PVT::ASSIGN_BITOR: return is(VT::O::ASS::B::OR) or is(VN::OP::ASS::BITOR);
        case PVT::ASSIGN_BITXOR: return is(VT::O::ASS::B::XOR) or is(VN::OP::ASS::BITXOR);
        case PVT::SCTOR: return tx == "sctor" or is(VN::OP::SCTOR);
        case PVT::LCTOR: return tx == "lctor" or is(VN::OP::LCTOR);
        case PVT::DTOR: return tx == "dtor" or is(VN::OP::DTOR);
        case PVT::CCTOR: return tx == "cctor" or is(VN::OP::CCTOR);
        case PVT::MCTOR: return tx == "mctor" or is(VN::OP::MCTOR);
        case PVT::AS: return is(VN::OP::AS);
        case PVT::POINT: return tx == "member" or is(VT::O::MEMBER);
        case PVT::SHARP: return tx == "aspect" or is(VT::O::SHARP);
        case PVT::MOVE: return tx == "move";
        case PVT::MEMBER: return is(VN::OP::MEMBER);
        case PVT::ASPECT: return is(VN::OP::ASPECT);

        case PVT::IS: return tx == "is";
        case PVT::WHILE: return tx == "while";
        case PVT::FOR: return tx == "for";
        case PVT::ON: return tx == "on" or is(VT::O::SC::COLON);
        case PVT::THEN: return tx == "then" or is(VT::O::SC::COLON);
        case PVT::WHEN: return tx == "when";
        case PVT::AFTER: return tx == "after";
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