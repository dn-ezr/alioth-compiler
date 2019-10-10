#ifndef __syntax_cpp__
#define __syntax_cpp__

#include "syntax.hpp"

namespace alioth {

node::node( $scope sc ):mscope(sc) {

}

bool node::isscope()const {
    return false;
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

bool eprototype::is( type t )const {
    return t == ELEPROTO;
}

bool aliasdef::is( type t )const {
    return t == ALIASDEF or t == DEFINITION;
}

bool classdef::is( type t )const {
    return t == CLASSDEF or t == DEFINITION;
}

bool enumdef::is( type t )const {
    return t == ENUMDEF or t == DEFINITION;
}

bool nameexpr::is( type t )const {
    return t == NAMEEXPR or t == EXPRESSION;
}

bool typeexpr::is( type t )const {
    return t == TYPEEXPR or t == EXPRESSION;
}
bool typeexpr::is_type( typeid_t t )const {
    return (id & t) != 0;
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

    return movi(1), constructModuleSignature(diagnostic);
}

$fragment SyntaxContext::constructFragment() {
    $fragment frag = new fragment;
    $definition def_padding;
    $implementation impl_padding;

    for( auto i = source.begin(); i != source.end(); i++ )
        if( i->is(VT::U::SPACE,VT::U::C::BLOCK,VT::U::C::LINE) )
            i.r.remove(i--.pos);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::R::BEG) ) {
                movi(2);
            } else if( it->is(VN::MODULE) ) {
                movi(3);
            } else {
                return diagnostics("41", *it), nullptr;
            } break;
        case 2:
            if( auto sig = constructModuleSignature(true); sig ) {
                redu(1, VN::MODULE);
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VT::LET) ) {
                def_padding = constructAliasDefinition(frag);
                if( !def_padding ) return nullptr;
            } else if( it->is(VT::CLASS) ) {
                auto def_padding = constructClassDefinition(frag);
                if( !def_padding ) return nullptr;
            } else if( it->is(VT::ENUM) ) {
                return not_ready_yet, nullptr;
            } else if( it->is(VT::OPERATOR) ) {
                return not_ready_yet, nullptr;
            } else if( it->is(VT::METHOD) ) {
                return not_ready_yet, nullptr;
            } else if( it->is(VN::CLASSDEF,VN::ALIASDEF,VN::ENUMDEF) ) {
                frag->defs << def_padding;
                stay();
            } else if( it->is(VT::R::END) ) {
                redu(2, VN::MODULE);
            } else {
                return diagnostics("40", *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    frag->phrase = *it;
    return frag;
}

$signature SyntaxContext::constructModuleSignature( bool diagnostic ) {
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
                auto im = constructDependencyDescriptor(sig, diagnostic);
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
        default:
            return internal_error, nullptr;
    }

    sig->phrase = *it;
    return sig;
}

$depdesc SyntaxContext::constructDependencyDescriptor( $scope scope, bool diagnostic ) {
    $depdesc ref = new depdesc;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(2);
            } else {
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
            if( it->is(VT::L::LABEL,VT::O::MEMBER,VT::L::STRING,VT::L::CHAR) ) {
                ref->from = *it;
                movi(4);
            } else {
                if( diagnostic ) diagnostics("25",*it);
                return nullptr;
            }
            break;
        case 4:
            if( it->is(VT::AS) ) {redu(-2,VN::LIST);}
            else redu(-4,VN::DEPENDENCY);
            break;
        case 5:
            if( it->is(VT::L::THIS) ) {
                ref->alias = *it;
                movi(6);
            } else if( it->is(VT::L::LABEL) ) {
                ref->alias = *it;
                redu(3,VN::DEPENDENCY);
            } else {
                if( diagnostic ) diagnostics("26",*it);
                return nullptr;
            } break;
        case 6:
            if( it->is(VT::MODULE) ) {
                redu(4,VN::DEPENDENCY);
            } else {
                if( diagnostic ) diagnostics("21",VT::MODULE,*it);
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$eprototype SyntaxContext::constructElementPrototype( $scope scope ) {
    $eprototype ref = new eprototype;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::ELETYPE,VT::VAR) ) {
                switch( it->id ) {
                    case VT::VAR: ref->etype = eprototype::var; break;
                    case VT::OBJ: ref->etype = eprototype::obj; break;
                    case VT::PTR: ref->etype = eprototype::ptr; break;
                    case VT::REF: ref->etype = eprototype::ref; break;
                    case VT::REL: ref->etype = eprototype::rel; break;
                    default: return internal_error, nullptr;
                }
                movi(2);
            } else if( it->is(VN::TYPEEXPR) ) {
                redu(1, VN::ELEPROTO);
            } else {
                ref->etype = eprototype::var;
                movi(2,0);
            } break;
        case 2:
            if( it->is(VN::TYPEEXPR) ) {
                if( ref->etype == eprototype::var and it->id == UnknownType )
                    return diagnostics("43", *it), nullptr;
                else if( ref->etype == eprototype::obj and ref->dtype->is_type(PointerTypeMask) )
                    return diagnostics("42", *it), nullptr;
                else if( ref->etype == eprototype::ptr and !ref->dtype->is_type(PointerTypeMask) )
                    return diagnostics("42", *it), nullptr;
                else
                    redu(1, VN::TYPEEXPR);
            } else if( auto t = constructTypeExpression(scope, true); t ) {
                ref->dtype = t;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$aliasdef SyntaxContext::constructAliasDefinition( $scope scope ) {
    $aliasdef ref = new aliasdef;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::LET) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::LET, *it ), nullptr;
            } break;
        case 2:
            if( it->is(PVT::PUBLIC,PVT::PROTECTED,PVT::PRIVATE) ) {
                if( ref->visibility ) diagnostics("27", *it );
                else ref->visibility = *it;
                stay(); // 以前这里是直接报错退出的，现在的设计希望报告尽可能多的错误
            } else if( it->is(VT::L::LABEL) ) {
                movi(3);
                ref->name = *it;
            } else {
                return diagnostics("28", *it ), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::ASSIGN) ) {
                movi(4);
            } else {
                return diagnostics("21", VT::O::ASSIGN, *it ), nullptr;
            } break;
        case 4:
            if( auto nm = constructNameExpression(scope, true); nm ) {
                ref->tagret = nm;
                redu(4, VN::ALIASDEF );
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$classdef SyntaxContext::constructClassDefinition( $scope scope ) {
    $classdef ref = new classdef;
    ref->setScope(scope);
    std::set<int> premise;
    $definition padding;
    classdef::predicate pred;
    bool branch = false;
    int step = 0;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::CLASS) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::CLASS, *it ), nullptr;
            } break;
        case 2:
            if( it->is(PVT::PRIVATE,PVT::PRIVATE,PVT::PUBLIC) ) {
                if( ref->visibility ) diagnostics("27", *it );
                else ref->visibility = *it;
                stay();
            } else if( it->is(PVT::ABSTRACT) ) {
                if( ref->abstract ) diagnostics("27", *it );
                else ref->abstract = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return diagnostics("28", *it ), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::LT) ) {
                if( step > 0 ) return diagnostics("34", *it), nullptr;
                movi(7);
            } else if( it->is(VT::O::SC::COLON) ) {
                if( step > 1 ) return diagnostics("35", *it), nullptr;
                movi(10);
            } else if( it->is(VT::O::SC::O::L) ) {
                if( ref->targf.size() == 0 ) return diagnostics("36", *it), nullptr;
                movi(11,0);
            } else if( it->is(VT::O::SC::O::S) ) {
                movi(4);
            } else if( it->is(VN::LIST) ) {
                step += 1;
                stay();
            } else {
                return diagnostics("21", VT::O::SC::O::S, *it ), nullptr;
            } break;
        case 4:
            if( it->is(VT::LET) ) {
                if( auto def = constructAliasDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(VT::CLASS) ) {
                if( auto def = constructClassDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(VT::ENUM) ) {
                return not_ready_yet, nullptr;
                //if( auto def = constructEnumerateDefinition(ref); def ) padding = def;
                //else return nullptr;
            } else if( it->is(VT::METHOD) ) {
                return not_ready_yet, nullptr;
                //if( auto def = constructMethodDefinition(ref); def ) padding = def;
                //else return nullptr;
            } else if( it->is(VT::OPERATOR) ) {
                return not_ready_yet, nullptr;
                //if( auto def = constructOperatorDefinition(ref); def ) padding = def;
                //else return nullptr;
            } else if( it->is(CT::ELETYPE) ) {
                return not_ready_yet, nullptr;
                //if( auto def = constructAttributeDefinition(ref); def ) padding = def;
                //else return nullptr;
            } else if( it->is(VN::ALIASDEF,VN::CLASSDEF,VN::ENUMDEF,VN::OPDEF,VN::METDEF,VN::ATTRDEF) ) {
                padding->premise = premise;
                if( !branch ) premise.clear();
                ref->contents << padding;
                stay();
            } else if( it->is(VT::L::I::N) ) {
                premise.insert(stol(it->tx));
                movi(5);
            } else if( it->is(VN::LIST) ) {
                stay();
            } else if( it->is(VT::O::SC::C::S) ) {
                if( branch ) {
                    branch = false;
                    stay();
                } else {
                    redu(4, VN::CLASSDEF);
                }
            } else {
                return diagnostics("33", *it ), nullptr;
            } break;
        case 5:
            if( it->is(VT::O::SC::COMMA) ) {
                movi(6);
            } else if( it->is(VT::O::SC::O::S) ) {
                branch = true;
                redu(1,VN::LIST);
            } else if( it->is(VT::O::SC::COLON) ) {
                branch = false;
                redu(1,VN::LIST);
            } else if( it->is(VN::ITEM) ) {
                stay();
            } else {
                return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
            } break;
        case 6:
            if( it->is(VT::L::I::N) ) {
                premise.insert(stol(it->tx));
                redu(1,VN::ITEM);
            } else {
                return diagnostics("32", *it ), nullptr;
            } break;
        case 7:
            if( it->is(VT::L::LABEL) ) {
                ref->targf << *it;
                movi(8);
            } else {
                return diagnostics("29", *it), nullptr;
            } break;
        case 8:
            if( it->is(VT::O::SC::COMMA) ) {
                movi(9);
            } else if( it->is(VT::O::GT) ) {
                redu(2, VN::LIST);
            } else if( it->is(VN::ITEM) ) {
                stay();
            } else {
                return diagnostics("21", VT::O::GT, *it), nullptr;
            } break;
        case 9:
            if( it->is(VT::L::LABEL) ) {
                ref->targf << *it;
                redu(1,VN::ITEM);
            } else {
                return diagnostics("29", *it), nullptr;
            } break;
        case 10:
            if( it->is(VT::L::LABEL) ) {
                if( auto n = constructNameExpression(ref,true); n ) {
                    ref->supers << n;
                } else {
                    return nullptr;
                }
            } else if( it->is(VN::NAMEEXPR) ) {
                stay();
            } else {
                if( ref->supers.size() == 0 )
                    return diagnostics("37", *it), nullptr;
                else
                    redu(-1, VN::LIST);
            } break;
        case 11:
            if( it->is(VT::O::SC::O::L) ) {
                pred.clear();
                movi(12);
            } else if( it->is(VN::LIST) ) {
                ref->preds << pred;
                stay();
            } else {
                redu(-1, VN::LIST);
            } break;
        case 12:
            if( it->is(VT::L::LABEL) ) {
                pred.construct(-1).targ = *it;
                movi(13);
            } else if( it->is(VN::ITEM) ) {
                pred[-1].vn = *it;
                movi(14);
            } else {
                return diagnostics("29", *it), nullptr;
            } break;
        case 13:
            if( it->is(VT::O::EQ,VT::O::NE,VT::O::SHR,VT::O::LT) ) {
                movi(it->id);
            } else {
                return diagnostics("39", *it), nullptr;
            } break;
        case 14:
            if( it->is(VT::AND) ) {
                movi(15);
            } else if( it->is(VN::ITEM) ) {
                pred[-1].vn = *it;
                stay();
            } else if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::LIST);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        case 15:
            if( it->is(VT::L::LABEL) ) {
                pred.construct(-1).targ = *it;
                movi(13);
            } else if( it->is(VN::ITEM) ) {
                redu(1,VN::ITEM);
            } else {
                return diagnostics("29", *it), nullptr;
            } break;
        case VT::O::NE:
            if( it->is(VT::OBJ) ) {
                pred[-1].rule = 1;
                redu(2, VN::ITEM);
            } else if( it->is(VT::PTR) ) {
                pred[-1].rule = 2;
                redu(2, VN::ITEM);
            } else if( it->is(VT::REF) ) {
                pred[-1].rule = 3;
                redu(2, VN::ITEM);
            } else if( it->is(VT::REL) ) {
                pred[-1].rule = 4;
                redu(2, VN::ITEM);
            } else {
                return diagnostics("38", *it), nullptr;
            } break;
        case VT::O::EQ:
            if( it->is(VT::OBJ) ) {
                pred[-1].rule = 5;
                redu(2, VN::ITEM);
            } else if( it->is(VT::PTR) ) {
                pred[-1].rule = 6;
                redu(2, VN::ITEM);
            } else if( it->is(VT::REF) ) {
                pred[-1].rule = 7;
                redu(2, VN::ITEM);
            } else if( it->is(VT::REL) ) {
                pred[-1].rule = 8;
                redu(2, VN::ITEM);
            } else {
                return diagnostics("38", *it), nullptr;
            } break;
        case VT::O::SHR:
            if( it->is(VT::OBJ) ) {
                pred[-1].rule = 9;
                redu(2, VN::ITEM);
            } else if( it->is(VT::PTR) ) {
                pred[-1].rule = 10;
                redu(2, VN::ITEM);
            } else {
                return diagnostics("38", *it), nullptr;
            } break;
        case VT::O::LT:
            if( it->is(VT::O::GT) ) {
                movi(it->id);
            } else {
                return diagnostics("21", VT::O::GT, *it), nullptr;
            } break;
        case VT::O::GT:
            if( auto t = constructTypeExpression(ref, true); t ) {
                if( t->id == UnknownType ) return diagnostics("31", *it), nullptr;
                pred[-1].type = t;
                pred[-1].rule = 11;
                redu(3, VN::ITEM);
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$enumdef SyntaxContext::constructEnumerateDefinition( $scope scope ) {
    $enumdef ref = new enumdef;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::ENUM) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::ENUM, *it), nullptr;
            } break;
        case 2:
            if( it->is(PVT::PRIVATE,PVT::PRIVATE,PVT::PUBLIC) ) {
                if( ref->visibility ) diagnostics("27", *it );
                else ref->visibility = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return diagnostics("28", *it ), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::O::S) ) {
                movi(4);
            } else {
                return diagnostics("21", VT::O::SC::O::S, *it), nullptr;
            } break;
        case 4:
            if( it->is(VT::L::LABEL) ) {
                for( auto& t : ref->items ) {
                    if( t.tx == it->tx ) {
                        diagnostics("44", *it);
                        diagnostics[-1]( diagnostics.prefix(), "45", *it);
                    }
                }
                ref->items << *it;
                stay();
            } else if( it->is(VT::O::SC::C::S) ) {
                redu(4, VN::ENUMDEF);
            } else {
                return diagnostics("46", *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$nameexpr SyntaxContext::constructNameExpression( $scope scope, bool absorb ) {
    $nameexpr ref = new nameexpr;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(2);
            } else {
                return diagnostics("29", *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::O::SCOPE) ) {
                movi(3);
            } else if( it->is(VT::O::LT) and absorb and ref->targs.size() == 0 ) {
                movi(4);
            } else if( it->is(VN::LIST) ) {
                stay();
            } else {
                redu(-2,VN::NAMEEXPR);
            } break;
        case 3:
            if( auto nm = constructNameExpression(scope, absorb); nm ) {
                ref->next = nm;
                redu(3,VN::NAMEEXPR);
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::ELEPROTO) ) {
                movi(5);
            } else if( auto t = constructElementPrototype(scope); t ) {
                ref->targs << t;
                if( t->dtype->id == UnknownType ) return diagnostics("31", *it ), nullptr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::O::SC::COMMA) ) {
                movi(6);
            } else if( it->is(VT::O::GT) ) {
                redu(2,VN::LIST);
            } else if( it->is(VT::O::SHR) ) {
                it->id = VT::O::GT;
                it->tx = VT::written_table.at(VT::O::GT);
                it.r.insert(*it,it.pos);
            } else if( it->is(VN::LIST) ) {
                stay();
            } else {
                return diagnostics("30", *it ), nullptr;
            } break;
        case 6:
           if( it->is(VN::ELEPROTO) ) {
                redu(1,VN::LIST);
            } else if( auto t = constructElementPrototype(scope); t ) {
                ref->targs << t;
                if( t->dtype->id == UnknownType ) return diagnostics("31", *it ), nullptr;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$typeexpr SyntaxContext::constructTypeExpression( $scope scope, bool absorb ) {
    $typeexpr ref = new typeexpr;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::BASIC_TYPE) ) {
                switch( it->id ) {
                    case VT::INT8: ref->id = Int8Type;
                    case VT::INT16: ref->id = Int16Type;
                    case VT::INT32: ref->id = Int32Type;
                    case VT::INT64: ref->id = Int64Type;
                    case VT::UINT8: ref->id = Uint8Type;
                    case VT::UINT16: ref->id = Uint16Type;
                    case VT::UINT32: ref->id = Uint32Type;
                    case VT::UINT64: ref->id = Uint64Type;
                    case VT::FLOAT32: ref->id = Float32Type;
                    case VT::FLOAT64: ref->id = Float64Type;
                    case VT::BOOL: ref->id = BooleanType;
                    case VT::VOID: ref->id = VoidType;
                    default: return internal_error, nullptr;
                }
                redu(1, VN::TYPEEXPR);
            } else if( it->is(VT::CLASS) ) {
                if( absorb ) diagnostics("27", *it );
                else absorb = true;
                stay();
            } else if( it->is(VT::O::MUL) ) {
                ref->id = UnconstraintedPointerType;
                movi(2);
            } else if( it->is(VT::O::XOR) ) {
                ref->id = ConstraintedPointerType;
                movi(2);
            } else if( it->is(VT::L::LABEL) ) {
                ref->id = NamedType;
                movi(3, 0);
            } else if( it->is(VT::L::THIS) ) {
                movi(4);
            } else {
                ref->id = UnknownType;
                redu(-1,VN::TYPEEXPR);
            } break;
        case 2:
            if( it->is(VN::TYPEEXPR) ) {
                redu(2, VN::TYPEEXPR);
            } else if( auto t = constructTypeExpression(scope, absorb); t ) {
                ref->sub = t;
                if( t->id == UnknownType ) 
                    return diagnostics("31", *it ), nullptr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VN::NAMEEXPR) ) {
                redu(2,VN::TYPEEXPR);
            } else if( auto n = constructNameExpression(scope, absorb); n ) {
                ref->sub = n;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VT::CLASS) ) {
                ref->id = ThisClassType;
                redu(2,VN::TYPEEXPR);
            } else {
                return diagnostics("21", VT::CLASS, *it ), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

}

#endif