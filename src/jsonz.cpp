#ifndef __json_cpp__
#define __json_cpp__

#include "jsonz.hpp"
#include <map>
#include <stdexcept>
#include <cctype>
#include "chainz.hpp"

using strty = std::string;
using arrty = chainz<json>;
using objty = std::map<strty,json>;

void json::clean() {
    switch( mtype ) {
        case boolean: delete (bool*)mdata;break;
        case integer: delete (long*)mdata;break;
        case number: delete (double*)mdata;break;
        case string: delete (strty*)mdata;break;
        case object: delete (objty*)mdata;break;
        case array: delete (arrty*)mdata;break;
        default : break;
    }
    mtype = null;
    mdata = nullptr;
}

json::json( type t ):mtype(t) {
    switch( t ) {
        case boolean : mdata = new bool(false);break;
        case integer : mdata = new long(0);break;
        case number : mdata = new double(0.0);break;
        case string : mdata = new strty();break;
        case object : mdata = new objty();break;
        case array : mdata = new arrty();break;
        case null : mdata = nullptr;break;
        default : throw std::invalid_argument("invalid type for json::json(type);");
    }
}

json::json( long x ):mtype(integer),mdata(new long(x)){}
json::json( double x ):mtype(number),mdata(new double(x)){}
json::json( const strty& s ):mtype(string),mdata(new strty(s)){}
json::json( strty&& s ):mtype(string),mdata(new strty(std::move(s))){}
json::json( bool b ):mtype(boolean),mdata(new bool(b)){}
json::json( json&& j ):mtype(j.mtype),mdata(j.mdata){j.mdata = nullptr;j.mtype = null;}
json::json( const json& j ):mtype(j.mtype) {
    switch( mtype ) {
        case boolean: mdata = new bool(*(bool*)j.mdata);break;
        case integer: mdata = new long(*(long*)j.mdata);break;
        case number: mdata = new double(*(double*)j.mdata);break;
        case string: mdata = new strty(*(strty*)j.mdata);break;
        case object: mdata = new objty(*(objty*)j.mdata);break;
        case array: mdata = new arrty(*(arrty*)j.mdata);break;
        case null: mdata = nullptr;break;
    }
}

json::~json() {
    clean();
}

json::type json::is()const{ return mtype; }
bool json::is( type t ) const { return mtype == t; }

json::operator bool &() {
    if( mtype != boolean ) throw std::runtime_error("cannot convert json to boolean");
    return *(bool*)mdata;
}
json::operator const bool &()const {
    if( mtype != boolean ) throw std::runtime_error("cannot convert json to boolean");
    return *(const bool*)mdata;
}

json::operator long&() {
    if( mtype != integer ) throw std::runtime_error("cannot convert json to integer");
    return *(long*)mdata;
}
json::operator const long&()const {
    if( mtype != integer ) throw std::runtime_error("cannot convert json to integer");
    return *(const long*)mdata;
}

json::operator double&() {
    if( mtype != number ) throw std::runtime_error("cannot convert json to number");
    return *(double*)mdata;
}
json::operator const double&()const {
    if( mtype != number ) throw std::runtime_error("cannot convert json to number");
    return *(const double*)mdata;
}

json::operator strty&() {
    if( mtype != string ) throw std::runtime_error("cannot convert json to string");
    return *(strty*)mdata;
}
json::operator const strty&()const {
    if( mtype != string ) throw std::runtime_error("cannot convert json to string");
    return *(const strty*)mdata;
}

json& json::operator[]( const strty& key ) {
    if( mtype != object ) throw std::runtime_error("cannot convert json to object");
    return (*(objty*)mdata)[key];
}
const json& json::operator[]( const strty& key )const {
    if( mtype != object ) throw std::runtime_error("cannot convert json to object");
    if( !((objty*)mdata)->count(key) ) throw std::out_of_range("attribute not exists");
    return (*(objty*)mdata)[key];
}

json& json::operator[]( int idx ) {
    if( mtype != array ) throw std::runtime_error("cannot convert json to array");
    auto& list = *(arrty*)mdata;

    if( idx >= 0 ) while( idx >= list.size() ) list << (null);
    else while( -idx > list.size() ) list.insert(null,0);
    return list[idx];
}

const json& json::operator[]( int idx ) const {
    if( mtype != array ) throw std::runtime_error("cannot convert json to array");
    auto& list = *(arrty*)mdata;
    auto ret = list.get(idx);
    if( !ret ) throw std::out_of_range("element not exists");
    return *ret;
}

int json::count( const strty& key )const {
    if( mtype != object ) throw std::runtime_error("cannot convert json to object");
    return ((objty*)mdata)->count(key);
}
int json::count( const strty& key, type t )const {
    if( mtype != object ) throw std::runtime_error("cannot convert json to object");
    if( ((objty*)mdata)->count(key) ) return (*this)[key].is(t);
    else return 0;
}
int json::count()const {
    if( mtype != array ) throw std::runtime_error("cannot convert json to array");
    return ((arrty*)mdata)->size();
}

void json::erase( const strty& key ){
    if( mtype != object ) throw std::runtime_error("cannot convert json to object");
    ((objty*)mdata)->erase(key);
}
void json::erase( int idx ) {
    if( mtype != array ) throw std::runtime_error("cannot convert json to array");
    auto& list = *(arrty*)mdata;
    list.remove(idx);
}

json& json::operator = ( bool b ) {
    if( mtype != boolean ) {clean();operator=(json(b));}
    else *(bool*)mdata = b;
    return *this;
}
json& json::operator = ( long x ) {
    if( mtype != integer ) {clean();operator=(json(x));}
    else *(long*)mdata = x;
    return *this;
}
json& json::operator = ( double x ) {
    if( mtype != number ) {clean();operator=(json(x));}
    else *(double*)mdata = x;
    return *this;
}
json& json::operator = ( const strty& s ) {
    if( mtype != string ) {clean();operator=(json(s));}
    else *(strty*)mdata = s;
    return *this;
}
json& json::operator = (strty&& s ) {
    if( mtype != string ) {clean();operator=(json(std::move(s)));}
    else *(strty*)mdata = std::move(s);
    return *this;
}
json& json::operator = ( const json& j ) {
    if( &j == this ) return *this;
    auto temp = j;
    clean();
    mtype = temp.mtype;
    mdata = temp.mdata;
    temp.mtype = null;
    temp.mdata = nullptr;
    return *this;
}
json& json::operator = ( json&& j ) {
    if( &j == this ) return *this;
    auto temp = std::move(j);
    clean();
    mtype = temp.mtype;
    mdata = temp.mdata;
    temp.mtype = null;
    temp.mdata = nullptr;
    return *this;
}

json json::FromJsonStream( std::istream& is ) {

    json data;
    strty* tk = nullptr;
    int state = 1;
    bool stay = false;

    strty err;

    while( state > 0 ) {
        switch( const char it = is.peek(); state ) {
            case 1:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == '\"' ) {
                    state = 2;
                    data = strty();
                    tk = (strty*)data.mdata;
                } else if( it == 'n' ) {
                    if( is.get() == 'n' and is.get() == 'u' and is.get() == 'l' and is.get() == 'l' ) {
                        stay = true;
                        state = 0;
                    } else {
                        state = -3;
                    }
                } else if( it == 'f' ) {
                    if( is.get() == 'f' and is.get() == 'a' and is.get() == 'l' and is.get() == 's' and is.get() == 'e' ) {
                        stay = true;
                        state = 0;
                        data = false;
                    } else {
                        state == -3;
                    }
                } else if( it == 't' ) {
                    if( is.get() == 't' and is.get() == 'r' and is.get() == 'u' and is.get() == 'e' ) {
                        stay = true;
                        state = 0;
                        data = true;
                    } else {
                        state == -3;
                    }
                } else if( it == '-' or isdigit(it) ) {
                    double r;
                    is >> r;
                    if( is.good() ) {
                        stay = true;
                        state = 0;
                        long x = r;
                        if( (r - x) == 0 ) data = x;
                        else data = r;
                    } else {
                        state == -3;
                    }
                } else if( it == '{' ) {
                    data = json(object);
                    state = 4;
                } else if( it == '[' ) {
                    data = json(array);
                    state = 10;
                } else {
                    state = -4;
                } break;
            case 2:
                if( it == EOF ) {
                    state = -1;
                } else if( it == '\"' ) {
                    tk = nullptr;
                    state = 0;
                } else if( it == '\\' ) {
                    state = 3;
                } else {
                    *tk += it;
                } break;
            case 3:
                if( it == EOF ) {
                    state = -1;
                } else if( it == '\"' ) {
                    *tk += '\"';
                } else if( it == '\\' ) {
                    *tk += '\\';
                } else if( it == '/' ) {
                    *tk += '/';
                } else if( it == 'b' ) {
                    *tk += '\b';
                } else if( it == 'n' ) {
                    *tk += '\n';
                } else if( it == 'r' ) {
                    *tk += '\r';
                } else if( it == 't' ) {
                    *tk += '\t';
                } else if( it == 'u' ) {
                    char buf[5];
                    is.get();
                    buf[0] = is.get();
                    buf[1] = is.get();
                    buf[2] = is.get();
                    buf[3] = is.get();
                    buf[4] = 0;
                    stay = true;
                    if( isxdigit(buf[0]) and isxdigit(buf[1]) and isxdigit(buf[2]) and isxdigit(buf[3]) ) {
                        int r = strtol(buf,nullptr,16);
                        *tk += r&0x00FF;
                        if( (r & 0xFF00) != 0 ) *tk += (r>>8);
                    } else {
                        state = -2;
                    }
                } else {
                    state = -2;
                }
                if( state > 0 ) state = 2;
                break;
            case 4:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == '\"' ) {
                    tk = new strty;
                    state = 5;
                } else if( it == '}' ) {
                    state = 0;
                } else {
                    state = -4;
                } break;
            case 5:
                if( it == EOF ) {
                    state = -1;
                } else if( it == '\"' ) {
                    state = 6;
                } else {
                    *tk += it;
                } break;
            case 6:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == ':' ) {
                    state = 7;
                } else {
                    state = -4;
                } break;
            case 7:
                data[*tk] = FromJsonStream(is);
                delete tk;
                tk = nullptr;
                state = 8;
                stay = true;
                break;
            case 8:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == ',' ) {
                    state = 9;
                } else if( it == '}' ) {
                    state = 0;
                } else {
                    state = -4;
                } break;
            case 9:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == '\"' ) {
                    tk = new strty;
                    state = 5;
                } else if( it == '}') {
                    state = -5;
                } else {
                    state = -4;
                } break;
            case 10:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == ']' ) {
                    state = 0;
                } else {
                    data[data.count()] = FromJsonStream(is);
                    stay = true;
                    state = 11;
                } break;
            case 11:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == ',' ) {
                    state = 12;
                } else if( it == ']' ) {
                    state = 0;
                } else {
                    state = -4;
                } break;
            case 12:
                if( it == EOF ) {
                    state = -1;
                } else if( isspace(it) ) {
                    //[do nothing]
                } else if( it == ']' ) {
                    state = -6;
                } else {
                    data[data.count()] = FromJsonStream(is);
                    stay = true;
                    state = 11;
                } break;
        }

        if( state < 0 ) {
            int count = 8;
            while( count-- > 0 and is.peek() != EOF ) err += is.get();
        }
        if( stay ) stay = false;
        else is.get();
    }

    if( tk ) delete tk;

    if( state == -1 ) throw std::runtime_error("unexpected EOF:" + err );
    else if( state == -2 ) throw std::runtime_error("invalid escape character:" + err );
    else if( state == -3 ) throw std::runtime_error("illegal constant:" + err );
    else if( state == -4 ) throw std::runtime_error("unexcepted illegal character:" + err );
    else if( state == -5 ) throw std::runtime_error("unexpected end of object:" + err );
    else if( state == -6 ) throw std::runtime_error("unexpected end of array:" + err );
    else if( state < 0 ) throw std::runtime_error("fail to parse json:" + err );
    else return data;
}

strty json::toJsonString() const {
    switch ( mtype ) {
        case null: return "null";
        case boolean: return *(bool*)mdata?"true":"false";
        case integer: return std::to_string(*(long*)mdata);
        case number: return std::to_string(*(double*)mdata);
        case object: {
            const auto& map = *(objty*)mdata;
            auto count = map.size();
            if( count == 0 ) return "{}";
            strty ret = "{";
            for( auto [key,value] : map ) {
                ret += "\"" + key + "\":" + value.toJsonString();
                if( count-- > 1 ) ret += ',';
            }
            ret += "}";
            return ret;
        }
        case array: {
            const auto& list = *(arrty*)mdata;
            auto count = list.size();
            if( count == 0 ) return "[]";
            strty ret = "[";
            for( auto value : list ) {
                ret += value.toJsonString();
                if( count-- > 1 ) ret += ',';
            }
            ret += "]";
            return ret;
        }
        case string: {
            strty ret = "\"";
            for( auto i = ((strty*)mdata)->begin(); i != ((strty*)mdata)->end(); i++ ) switch( *i ) {
                case '\"': ret += "\\\"";break;
                case '\\': ret += "\\\\";break;
                //case '/': ret += "\\/";break;
                case '\b': ret += "\\b";break;
                case '\n': ret += "\\n";break;
                case '\r': ret += "\\r";break;
                default:
                    /*if( isprint(*i) )*/ ret += *i;
                    /*else if( *(i+1) != '\0' ) {
                        char buf[8];
                        sprintf(buf,"%02x%02x", 0x00FF&*(i+1), 0x00FF&*i );
                        ret += "\\u" + strty(buf);
                        i += 1;
                    } else {
                        char buf[8];
                        sprintf(buf,"00%02x", 0x00FF&*i );
                        ret += "\\u" + strty(buf);
                    }*/
            }
            ret += "\"";
            return ret;
        }
        default:
            throw std::runtime_error("internal error of json type");
    }
}

chainz<json>::iterator json::begin() {
    if( is(array) )
        return ((arrty*)mdata)->begin();
    throw std::runtime_error("json::begin(): bad json value type.");
}

chainz<json>::citerator json::begin() const {
    if( is(array) )
        return ((const arrty*)mdata)->begin();
    throw std::runtime_error("json::begin()const: bad json value type.");
}

chainz<json>::iterator json::end() {
    if( is(array) )
        return ((arrty*)mdata)->end();
    throw std::runtime_error("json::end(): bad json value type.");
}

chainz<json>::citerator json::end() const {
    if( is(array) )
        return ((const arrty*)mdata)->end();
    throw std::runtime_error("json::end()const: bad json value type.");
}

int json::for_each( const std::function<bool(const std::string&, json&)>& fun ) {
    if( !is(object) ) throw std::runtime_error("json::for_each( const std::function<bool(const std::string& key, json&)>& fun ): bad json value type.");
    int count = 0;
    for( auto [key,value] : *(objty*)mdata ) {
        count += 1;
        if( !fun(key,value) ) break;
    }
    return count;
}
int json::for_each( const std::function<bool(const std::string&, const json&)>& fun ) const {
    if( !is(object) ) throw std::runtime_error("json::for_each( const std::function<bool(const std::string& key, const json&)>& fun )const: bad json value type.");
    int count = 0;
    for( auto [key,value] : *(const objty*)mdata ) {
        count += 1;
        if( !fun(key,value) ) break;
    }
    return count;
}

#endif