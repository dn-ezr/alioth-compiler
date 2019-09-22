#ifndef __syntax_cpp__
#define __syntax_cpp__

#include "syntax.hpp"

namespace alioth {

node::node( $scope sc ):mscope(sc) {

}

bool node::setScope( $scope sc ) {
    if( mscope and sc ) return false;
    mscope = sc;
    return true;
}

$scope node::getScope()const {
    return mscope;
}

Uri node::getDocUri() {
    return mscope?mscope->getDocUri():Uri::Bad;
}

$fragment node::getFragment() {
    return mscope?mscope->getFragment():nullptr;
}

$module node::getModule() {
    return mscope?mscope->getModule():nullptr;
}

bool signature::is( type t )const {
    return t == SIGNATURE;
}
bool signature::isscope()const {
    return true;
}

json signature::toJson() const {
    json obj = json::object;
    obj["name"] = name;
    if( entry ) obj["entry"] = entry;
    auto& jdeps = obj["deps"] = json(json::array);
    auto& jcode = obj["code"] = json(json::array);
    for( auto dep : deps ) jdeps[jdeps.count()] = dep->toJson();
    for( auto& [code,_] : docs ) {
        auto c = code;
        c.flags = SpaceEngine::HideMain(c.flags);
        c.package.clear();
        jcode[jcode.count()] = c.toJson();
    }
    return obj;
}

$signature signature::fromJson( const json& object, srcdesc space ) {
    if( !object.is(json::object) ) return nullptr;
    $signature sig = new signature;
    sig->space = space;
    if( !object.count("name",json::string) ) return nullptr;
    sig->name = (string)object["name"];
    if( !sig->name.islabel() ) return nullptr;

    if( object.count("entry",json::string) ) {
        sig->entry = (string)object["entry"];
        if( !sig->entry.islabel() ) return nullptr;
    }
    if( object.count("deps",json::array) ) 
        for( auto& dep : object["deps"] ) {
            auto desc = depdesc::fromJson(dep);
            if( !desc ) continue;
            desc->setScope(sig);
            sig->deps << desc;
        }
    if( object.count("code",json::array) ) 
        for( auto& desc : object["code"] ) {
            auto d = fulldesc::fromJson(desc);
            if( !d ) continue;
            d.flags |= space.flags;
            d.package = space.package;
            sig->docs[d] = nullptr;
        }
    return sig;
}

bool signature::combine( $signature an ) {
    if( !an or an->name != name or an->space != space ) return false;
    for( auto[k,v] : an->docs ) docs[k] = v;
    for( auto dep : an->deps ) {
        dep->setScope(nullptr);
        dep->setScope(this);
    }
    deps += an->deps;
    return true;
}

bool depdesc::is( type t )const {
    return t == DEPDESC;
}
bool depdesc::isscope()const {
    return false;
}

json depdesc::toJson()const {
    json dep = json::object;
    dep["name"] = name.tx;
    dep["from"] = get<1>(from.extractContent());
    dep["alias"] = alias.tx;
    return dep;
}

$depdesc depdesc::fromJson( const json& object ) {
    $depdesc desc = new depdesc;
    if( !object.is(json::object) ) return nullptr;
    
    if( object.count("name",json::string) )
        desc->name = (string)object["name"];
    else
        return nullptr;
    if( object.count("alias",json::string) ) {
        desc->alias = (string)object["alias"];
        if(desc->alias.tx == "this") 
            desc->alias.id = VT::L::THIS; }
    
    if( object.count("from",json::string) ) {
        desc->from = (string)object["from"];
        if(desc->from.tx == ".") 
            desc->alias.id = VT::O::MEMBER; }

    return desc;
}

bool fragment::is( type t )const {
    return t == FRAGMENT;
}
bool fragment::isscope()const {
    return true;
}

SyntaxContext::state::state(int _s,int _c):s(_s),c(_c) {

}
SyntaxContext::state::operator int&() {
    return s;
}

void SyntaxContext::movi(int s,int c) {
    states << state(s,c);
    it += c;
}

void SyntaxContext::movo(int c ) {
    while( c-- > 0 ) {
        it -= states[-1].c;
        states.pop();
    }

}

void SyntaxContext::stay(int c ) {
    states[-1].c += c;
    it += c;
}
void SyntaxContext::redu(int c, int n ) {
    token node = token(n);
    if( c >= 0 ) {
        //node.insert( std::move(*it), 0 );
        node.bl = it->bl;
        node.bc = it->bc;
        node.el = it->el;
        node.ec = it->ec;
        node.tx = it->tx;
        it.r.remove(it.pos);
    }
    else {
        c = -c;
        if( it.pos > 0 ) {
            node.bl = (it-1)->el;       //由于还不能确定状态中是否包含这个单词,所以不能直接囊括其坐标
            node.bc = (it-1)->ec;
            node.el = (it-1)->el;
            node.ec = (it-1)->ec;
            //node.tx = (it-1)->tx;
        }
    }

    while( c-- > 0 ) {
        while( states[-1].c-- > 0 ) {
            //node.insert( std::move(*(--it)), 0 );
            node.bl = (it-1)->bl;
            node.bc = (it-1)->bc;
            if( isalnum(node.tx[0]) and isalnum((it-1)->tx.back()) ) node.tx = (it-1)->tx + " " + node.tx;
            else node.tx = (it-1)->tx + node.tx;
            (--it).r.remove(it.pos);
        }
        states.remove(-1);
    }
    
    it.r.insert(std::move(node),it.pos);
}

SyntaxContext::wb SyntaxContext::enter() {
    ws << states.size();
    return std::move(wb(states.size(),ws.size(),*this));
}

bool SyntaxContext::working() {
    if( auto sub = states.size() - ws[-1]; sub > 0 ) return true;
    else return movo(sub), ws.pop(), false;
}

SyntaxContext::SyntaxContext( tokens& src, Diagnostics& ds ): source(src),diagnostics(ds),it(src.begin()) {
}

$signature SyntaxContext::extractSignature( bool diagnostic ) {
    for( auto i = source.begin(); i != source.end(); i++ )
        if( i->is(VT::U::SPACE,VT::U::C::BLOCK,VT::U::C::LINE) )
            i.r.remove(i--.pos);

    return movi(1), constructModuleSignature(nullptr,diagnostic);
}

$signature SyntaxContext::constructModuleSignature( $scope scope, bool diagnostic ) {
    $signature sig = nullptr;
    enter();

    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( !it->is(VT::MODULE) ) {
                if( diagnostic )  diagnostics("21",VT::MODULE,*it);
                return nullptr;
            }
            movi(2);
            break;
        case 2:
            if( it->is(VT::L::LABEL) ) {
                sig = new signature();
                sig->name = *it;
                movi(3);
            } else {
                if( diagnostic ) diagnostics("22",*it);
                return nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::COLON) ) movi(4);
            else if( it->is(VT::ENTRY) ) movi(5);
            else if( it->is(VN::LIST) ) redu(3,VN::MODULE);
            else redu(-3,VN::MODULE);
            break;
        case 4:
            if( it->is(VT::L::LABEL) ) {
                auto im = constructDependencyDescriptor(sig);
                if( !im ) return nullptr;
                sig->deps << im;
            } else if( it->is(VN::DEPENDENCY) ) {
                stay(1);
            } else if( it->is(VT::O::SC::SEMI) ) {
                redu(1,VN::LIST);
            } else {
                redu(-1,VN::LIST);
            } break;
        case 5:
            if( it->is(VT::L::LABEL) ) {
                sig->entry = *it;
                movi(6);
            } else {
                if( diagnostic ) diagnostics("23",*it);
                return nullptr;
            } break;
        case 6:
            if( it->is(VT::O::SC::COLON) ) movi(4);
            else if( it->is(VN::LIST) ) redu(5,VN::MODULE);
            else redu(-5,VN::MODULE);
            break;
    }

    sig->phrase = *it;
    sig->setScope(scope);
    return sig;
}

$depdesc SyntaxContext::constructDependencyDescriptor( $scope scope, bool diagnostic ) {
    $depdesc ref = new depdesc;
    enter();

    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::L::LABEL) ) {ref->name = *it;movi(2);}
            else {
                if( diagnostic ) diagnostics("24",*it);
                return nullptr;
            }
            break;
        case 2:
            if( it->is(VT::O::AT) ) movi(3);
            else if( it->is(VT::AS) ) movi(5);
            else if( it->is(VN::LIST) ) stay();
            else redu(-2,VN::DEPENDENCY);
            break;
        case 3:
            if( it->is(VT::L::LABEL,VT::O::MEMBER,VT::L::STRING,VT::L::CHAR) ) {ref->from = *it;movi(4);}
            else {
                if( diagnostic ) diagnostics("25",*it);
                return nullptr;
            }
            break;
        case 4:
            if( it->is(VT::AS) ) {redu(-2,VN::LIST);}
            else redu(-4,VN::DEPENDENCY);
            break;
        case 5:
            if( it->is(VT::L::THIS) ) {ref->alias = *it;movi(6);}
            else if( it->is(VT::L::LABEL) ) {ref->alias = *it;redu(3,VN::DEPENDENCY);}
            else {
                if( diagnostic ) diagnostics("26",*it);
                return nullptr;
            }
            break;
        case 6:
            if( it->is(VT::MODULE) ) {redu(4,VN::DEPENDENCY);}
            else {
                if( diagnostic ) diagnostics("21",VT::MODULE,*it);
                return nullptr;
            }
    }

    ref->phrase = *it;
    ref->setScope(scope);
    return ref;
}

}

#endif