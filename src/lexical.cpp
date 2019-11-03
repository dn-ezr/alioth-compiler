#ifndef __lexical_cpp__
#define __lexical_cpp__

#include "lexical.hpp"

namespace alioth {

LexicalContext::LexicalContext( istream& is, bool limit ):
  source(is),heading(limit) {
}

tokens LexicalContext::perform() {

    state = 1;
    stay = false;
    synst = 1;
    T = token(VT::R::BEG);
    ret.clear();
    ret << std::move(T);
    T.bl = begl = 1;
    T.bc = begc = 1;
    
    for(pre = source.peek(); state > 0; goon() ) switch( state ) {
        case 1:
            if( pre == EOF ) state = 0;
            else if( isspace(pre) ) state = 3;
            else if( pre == 'a' ) state = 10;
            else if( pre == 'b' ) state = 11;
            else if( pre == 'c' ) state = 12;
            else if( pre == 'd' ) state = 16;
            else if( pre == 'e' ) state = 18;
            else if( pre == 'f' ) state = 20;
            else if( pre == 'i' ) state = 22;
            else if( pre == 'l' ) state = 24;
            else if( pre == 'm' ) state = 25;
            else if( pre == 'n' ) state = 27;
            else if( pre == 'o' ) state = 28;
            else if( pre == 'p' ) state = 29;
            else if( pre == 'r' ) state = 30;
            else if( pre == 's' ) state = 32;
            else if( pre == 't' ) state = 33;
            else if( pre == 'u' ) state = 34;
            else if( pre == 'v' ) state = 37;
            else if( pre == 'x' ) state = 59;
            else if( islabelb(pre) ) state = 4;
            else if( pre == '~' ) check(VT::O::B::REV,true);
            else if( pre == '!' ) state = 57;//assign(VT::NE,VT::FORCE);
            else if( pre == '@' ) check(VT::O::AT,true);
            else if( pre == '#' ) check(VT::O::SHARP,true);
            else if( pre == '$' ) check(VT::O::CONV,true);
            else if( pre == '%' ) assign(VT::O::ASS::MOL,VT::O::MOL);
            else if( pre == '^' ) assign(VT::O::ASS::B::XOR,VT::O::B::XOR);
            else if( pre == '&' ) assign(VT::O::ASS::B::AND,VT::O::B::AND);
            else if( pre == '*' ) assign(VT::O::ASS::MUL,VT::O::MUL);
            else if( pre == '(' ) check(VT::O::SC::O::A,true);
            else if( pre == ')' ) check(VT::O::SC::C::A,true);
            else if( pre == '-' ) state = 38;
            else if( pre == '+' ) state = 39;
            else if( pre == '=' ) state = 60; // assign(VT::O::EQ,VT::O::ASSIGN);
            else if( pre == '[' ) check(VT::O::SC::O::L,true);
            else if( pre == ']' ) check(VT::O::SC::C::L,true);
            else if( pre == '{' ) check(VT::O::SC::O::S,true);
            else if( pre == '}' ) check(VT::O::SC::C::S,true);
            else if( pre == '|' ) assign(VT::O::ASS::B::OR,VT::O::B::OR);
            else if( pre == ';' ) check(VT::O::SC::SEMI,true);
            else if( pre == ':' ) state = 40;
            else if( pre == ',' ) check(VT::O::SC::COMMA,true);
            else if( pre == '?' ) check(VT::O::ASK,true);
            else if( pre == '<' ) state = 41;
            else if( pre == '>' ) state = 42;
            else if( pre == '.' ) state = 43;
            else if( pre == '/' ) state = 45;
            else if( pre == '"' ) sequence(VT::L::STRING);
            else if( pre == '\'') sequence(VT::L::CHAR);
            else if( pre == '0' ) state = 9;
            else if( pre >= '1' and pre <= '9' ) {T.id = VT::L::I::N; state = 50;}
            else check(VT::R::ERR,true);
            break;
        case 2:
            if( pre and pre == fixed[off] ) off += 1;
            else if( islabel(pre) ) state = 4;
            else if( off == (long long)fixed.size() ) check(T.id,false);
            else check(VT::L::LABEL,false);
            break;
        case 3:
            if( !isspace(pre) ) check(VT::U::SPACE,false); 
            break;
        case 4:
            if( !islabel(pre) ) check(VT::L::LABEL,false);
            break;
        case 5:
            if( pre and pre == fixed[off] ) off += 1;
            else if( off == (long long)fixed.size() ) {state = target;stay = true;}
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 6:
            if( pre == '=' ) check(hit,true);
            else check(T.id,false);
            break;
        case 7:
            if( T.id == VT::L::STRING and pre == '\"' ) check(T.id,true);
            else if( T.id == VT::L::CHAR and pre == '\'' ) check(T.id,true);
            else if( pre == '\0' or pre == EOF ) check(VT::R::ERR,false);
            else if( pre == '\\' ) state = 8;
            break;
        case 8:
            if( pre == '\0' or pre == EOF ) check(VT::R::ERR,false);
            else state = 7;
            break;
        case 9:
            if( pre == 'b') {T.id = VT::L::I::B; state = 52;}
            else if( pre == 'o' ) {T.id = VT::L::I::O; state = 51;}
            else if( pre == 'x' ) {T.id = VT::L::I::H; state = 49;}
            else {T.id = VT::L::I::N; state = 50;stay = true;}
            break;
        case 10:
            if( pre == 's' ) state = 15;
            else if( pre == 'n' ) test(VT::O::AND,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 11:
            if( pre == 'r' ) test(VT::BREAK,1);
            else if( pre == 'o' ) test(VT::BOOL,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 12:
            if( pre == 'o' ) state = 13;
            else if( pre == 'l' ) test(VT::CLASS,1);
            else if( pre == 'a' ) test(VT::CASE,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 13:
            if( pre == 'n' ) state = 14;
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 14:
            if( pre == 's' ) test(VT::CONST,3);
            else if( pre == 't' ) test(VT::CONTINUE,3);
            else check(VT::L::LABEL,false);
            break;
        case 15:
            if( pre == 's' ) test(VT::ASSUME,2);
            else if( pre == '!' ) check(VT::O::TREAT,true);
            else if( islabel(pre) ) state = 4;
            else check(VT::O::AS,false);
            break;
        case 16:
            if( pre == 'e' ) state = 17;
            else if( pre == 'o' ) test(VT::DO, 1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 17:
            if( pre == 'l' ) test(VT::DELETE,2);
            else if( pre == 'f' ) test(VT::DEFAULT,2);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 18:
            if( pre == 'n' ) state = 19;
            else if( pre == 'l' ) test(VT::ELSE,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 19:
            if( pre == 'u' ) test(VT::ENUM,2);
            else if( pre == 't' ) test(VT::ENTRY,2);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 20:
            if( pre == 'a' ) test(VT::L::FALSE,1);
            else if( pre == 'l' ) prefix("float",1,21);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 21:
            if( pre == '3' ) test(VT::FLOAT32,5);
            else if( pre == '6' ) test(VT::FLOAT64,5);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 22:
            if( pre == 'f' ) test(VT::IF,1);
            else if( pre == 'n' ) prefix("int",1,23);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 23:
            if( pre == '8' ) test(VT::INT8,3);
            else if( pre == '1' ) test(VT::INT16,3);
            else if( pre == '3' ) test(VT::INT32,3);
            else if( pre == '6' ) test(VT::INT64,3);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 24:
            if( pre == 'e' ) test(VT::LET,1);
            else if( pre == 'o' ) test(VT::LOOP,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 25:
            if( pre == 'e' ) prefix("met",1,58);
            else if( pre == 'o' ) prefix("mod",1,26);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 26:
            if( pre == 'u' ) test(VT::MODULE,3);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 27:
            if( pre == 'e' ) test(VT::NEW,1);
            else if( pre == 'o' ) test(VT::O::NOT,1);
            else if( pre == 'u' ) test(VT::L::NULL,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 28:
            if( pre == 'b' ) state = 36;
            else if( pre == 'r' ) test(VT::O::OR,1);
            else if( pre == 't' ) test(VT::OTHERWISE,1);
            else if( pre == 'p' ) test(VT::OPERATOR,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 29:
            if( pre == 't' ) test(VT::PTR,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 30:
            if( pre == 'e' ) state = 31;
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 31:
            if( pre == 'f' ) test(VT::REF,2);
            else if( pre == 'l' ) test(VT::REL,2);
            else if( pre == 't' ) test(VT::RETURN,2);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 32:
            if( pre == 'w' ) test(VT::SWITCH,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 33:
            if( pre == 'h' ) test(VT::L::THIS,1);
            else if( pre == 'r' ) test(VT::L::TRUE,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 34:
            if( pre == 'i' ) prefix("uint",1,35);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 35:
            if( pre == '8' ) test(VT::UINT8,4);
            else if( pre == '1' ) test(VT::UINT16,4);
            else if( pre == '3' ) test(VT::UINT32,4);
            else if( pre == '6' ) test(VT::UINT64,4);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 36:
            if( pre == 'j' ) test(VT::OBJ,2);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 37:
            if( pre == 'a' ) test(VT::VAR,1);
            else if( pre == 'o' ) test(VT::VOID,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 38:
            if( pre == '-' ) check(VT::O::DECREMENT,true);
            else if( pre == '=' ) check(VT::O::ASS::MINUS,true);
            else if( pre == '>' ) check(VT::O::POINTER,true);
            else check(VT::O::MINUS,false);
            break;
        case 39:
            if( pre == '+' ) check(VT::O::INCREMENT,true);
            else if( pre == '=' ) check(VT::O::ASS::PLUS,true);
            else check(VT::O::PLUS,false);
            break;
        case 40:
            if( pre == ':' ) check(VT::O::SCOPE,true);
            else check(VT::O::SC::COLON,false);
            break;
        case 41:
            if( pre == '<' ) assign(VT::O::ASS::SHL,VT::O::SHL);
            else if( pre == '=' ) check(VT::O::LE,true);
            else check(VT::O::LT,false);
            break;
        case 42:
            if( pre == '>' ) assign(VT::O::ASS::SHR,VT::O::SHR);
            else if( pre == '=' ) check(VT::O::GE,true);
            else check(VT::O::GT,false);
            break;
        case 43:
            if( pre == '.' ) state = 44;
            else check(VT::O::MEMBER,false);
            break;
        case 44:
            if( pre == '.' ) check(VT::O::ETC,true);
            else check(VT::O::RANGE,false);
            break;
        case 45:
            if( pre == '/' ) state = 46;
            else if( pre == '*' ) state = 47;
            else if( pre == '=' ) check(VT::O::ASS::DIV,true);
            else check(VT::O::DIV,false);
            break;
        case 46:
            if( pre == '\n' or pre == EOF or pre == '\0' ) check(VT::U::C::LINE,false);
            break;
        case 47:
            if( pre == '*' ) state = 48;
            else if( pre == EOF or pre == '\0' ) check(VT::R::ERR,false);
            break;
        case 48:
            if( pre == '/' ) check(VT::U::C::BLOCK,true);
            else if( pre != '*' ) state = 47;
            else if( pre == EOF or pre == '\0' ) check(VT::R::ERR,false);
            break;
        case 49:
            if( (pre >= 'a' and pre <= 'f') or (pre >= 'A' and pre <= 'F') ) continue;
            [[fallthrough]];
        case 50:
            if( pre == '8' or pre == '9' ) continue;
            else if( pre == '.' ) {state = 53;break;}
            else if( pre == 'e' ) {state = 55;break;}
            [[fallthrough]];
        case 51:
            if( pre >= '2' and pre <= '7' ) continue;
            [[fallthrough]];
        case 52:
            if( pre == '0' or pre == '1' or pre == '\'' ) continue;
            else if( isalpha(pre) ) check(VT::R::ERR,true);
            else check(T.id,false);
            break;
        case 53:
            if( pre >= '0' and pre <= '9' ) state = 54;
            else check(VT::R::ERR,true);
            break;
        case 54:
            if( pre >= '0' and pre <= '9' ) continue;
            else if( pre == 'e' ) state = 55;
            else if( T.tx.find('\'') == std::string::npos ) check(VT::L::FLOAT,false);
            else check(VT::L::FLOAT,false);
            break;
        case 55:
            if( pre == '-' or pre == '+' ) state = 56;
            else if( pre >= '0' and pre <= '9' ) state = 56;
            else check(VT::R::ERR,true);
            break;
        case 56:
            if( pre >= '0' and pre <= '9' ) continue;
            else if( T.tx.find('\'') == std::string::npos ) check(VT::L::FLOAT,false);
            else check(VT::R::ERR,false);
            break;
        case 57:
            if( pre == '=' ) check(VT::O::NE,true);
            else if( pre == '!' ) check(VT::O::EXCEPTION,true);
            else check(VT::O::FORCE,false);
            break;
        case 58:
            if( pre == 'a' ) test(VT::META,3);
            else if( pre == 'h' ) test(VT::METHOD,3);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 59:
            if( pre == 'o' ) test(VT::O::XOR,1);
            else if( islabel(pre) ) state = 4;
            else check(VT::L::LABEL,false);
            break;
        case 60:
            if( pre == '=' ) check(VT::O::EQ, true);
            else if( pre == '>' ) check(VT::O::GENERATE, true);
            else check(VT::O::ASSIGN, false);
            break;
    }

    if( state < 0 ) {
        ret[-1].id = VT::R::END;
    } else {
        ret << token(VT::R::END);
        ret[-1].bl = ret[-1].el = begl;
        ret[-1].bc = ret[-1].ec = begc;
    }
    return std::move(ret);
}



void LexicalContext::goon() {
    if( stay ) {stay = false;return;}
    if( pre == '\n' ) begl += begc = 1;
    else begc += 1;
    T.tx += source.get();
    pre = source.peek();
};

void LexicalContext::check(int t,bool s ) {
    if( s ) goon();
    
    T.id = t;
    T.el = begl;
    T.ec = begc;
    ret << std::move(T);

    T.tx.clear();
    T.bl = begl;
    T.bc = begc;
    state = ((pre==EOF)?0:1);
    stay = true;

    /*微型语法分析器*/
    if( heading ) switch(t) {
        case VT::U::SPACE:case VT::U::C::LINE:case VT::U::C::BLOCK: break;
        case VT::MODULE:
            if( synst == 1 ) synst = 2;
            else if( synst == 9 ) synst = 5;
            else state = -1;
            break;
        case VT::L::LABEL:
            if( synst == 2 ) synst = 3;
            else if( synst == 4 ) synst = 5;
            else if( synst == 5 ) synst = 5;
            else if( synst == 6 ) synst = 7;
            else if( synst == 7 ) synst = 5;
            else if( synst == 8 ) synst = 5;
            else if( synst == 10 ) synst = 11;
            else state = -1;
            break;
        case VT::O::SC::COLON:
            if( synst == 3 ) synst = 4;
            else if( synst == 11 ) synst = 4;
            else state = -1;
            break;
        case VT::O::AT:
            if( synst == 5 ) synst = 6;
            else state = -1;
            break;
        case VT::O::MEMBER:case VT::L::CHAR:case VT::L::STRING:
            if( synst == 6 ) synst = 7;
            else state = -1;
            break;
        case VT::O::AS:
            if( synst == 5 ) synst = 8;
            else if( synst == 7 ) synst = 8;
            else state = -1;
            break;
        case VT::L::THIS:
            if( synst == 8 ) synst = 9;
            else state = -1;
            break;
        case VT::ENTRY:
            if( synst == 3 ) synst = 10;
            else state = -1;
            break;
        default:state = -1;
    }
};

void LexicalContext::test(int v, int o ){
    T.id = v;
    fixed = VT::written_table.at(v);
    off = o+1;
    state = 2;      //用于单词测试的状态
};

void LexicalContext::prefix( const char* s, int o, int t ) {
    fixed = s;
    off = o+1;
    target = t;
    state = 5;      //用于前缀测试的状态
};

void LexicalContext::assign(int h, int m) {
    T.id = m;
    hit = h;
    state = 6;      //用于赋值测试的状态
};

void LexicalContext::sequence( int h ) {
        T.id = h;
        state = 7;
}

bool LexicalContext::islabel( int c ) {
    return isalnum(c) or c == '_';
}
bool LexicalContext::islabelb( int c ) {
    return isalpha(c) or c == '_';
}

}

#endif