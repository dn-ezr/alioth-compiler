#ifndef __syntax_cpp__
#define __syntax_cpp__

#include "syntax.hpp"
#include "context.hpp"

namespace alioth {

#define clone_node(T,scope)\
    $##T ret = new T;\
    ret->phrase = phrase;\
    ret->setScope(scope);

#define clone_definition(T,scope)\
    clone_node(T,scope)\
    ret->name = name;\
    ret->visibility = visibility;\
    ret->premise = premise;

#define clone_implementation(T,scope)\
    clone_node(T,scope)\
    ret->host = host->clone(scope);\
    ret->name = name;\
    if( body ) ret->body = body->clone(ret);

#define clone_callable(scope)\
    if( ret_proto ) ret->ret_proto = ret_proto->clone(scope);\
    for( auto arg : arguments ) ret->arguments << ($element)arg->clone(scope);\
    ret->va_arg = va_arg;

#define clone_metprototype(scope)\
    clone_callable(scope)\
    ret->cons = cons;\
    ret->mode = mode;\
    ret->meta = meta;

#define clone_opprototype(scope)\
    clone_callable(scope)\
    ret->modifier = modifier;\
    ret->cons = cons;\
    ret->subtitle = subtitle;

#define clone_statement(T, scope)\
    clone_node(T, scope)\
    ret->name = name;

#define clone_expression(T, scope)\
    clone_statement(T,scope)\
    ret->etype = etype;

node::node( $scope sc ):mscope(sc) {

}

bool node::isscope() const {
    return false;
}

bool node::setScope( $scope sc ) {
    if( mscope and sc ) return false;
    mscope = sc;
    return true;
}

$scope node::getScope() const {
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

CompilerContext& node::getCompilerContext() {
    return mscope->getCompilerContext();
}
signature::entry_t::operator bool()const {
    return mark and doc;
}

json signature::entry_t::toJson()const {
    json ret = json::object;
    ret["doc"] = doc.toJson();
    ret["mark"] = mark;
    return ret;
}

signature::entry_t signature::entry_t::fromJson( const json& obj ) {
    entry_t ret;
    if( !obj.is(json::object) ) return ret;
    if( obj.count("doc", json::object) ) ret.doc = srcdesc::fromJson(obj["doc"]);
    if( obj.count("mark", json::string) ) ret.mark = (string)obj["mark"];
    return ret;
}

CompilerContext& signature::getCompilerContext() {
    return *context;
}
bool signature::is( type t ) const {
    return t == SIGNATURE;
}
$node signature::clone( $scope scope ) const {
    throw logic_error("signature::clone(): This method is not allowed");
}
json signature::toJson() const {
    json obj = json::object;
    obj["name"] = name;
    if( entry ) obj["entry"] = entry.toJson();
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

    if( object.count("entry",json::object) ) {
        sig->entry = entry_t::fromJson(object["entry"]);
    }
    if( object.count("deps",json::array) ) 
        for( auto& dep : object["deps"] ) {
            auto desc = depdesc::fromJson(dep, space);
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
            sig->docs[d] = {};
        }
    return sig;
}

bool signature::combine( $signature an ) {
    if( !an or an->name != name or an->space != space ) return false;
    if( an->entry ) {
        if( entry and an->entry.doc != entry.doc ) return false;
        else entry = an->entry;
    }
    for( auto[k,v] : an->docs ) docs[k] = v;
    for( auto dep : an->deps ) {
        dep->setScope(nullptr);
        dep->setScope(this);
    }
    deps += an->deps;
    return true;
}

bool depdesc::is( type t ) const {
    return t == DEPDESC;
}

$node depdesc::clone( $scope scope ) const {
    throw logic_error("depdesc::clone(): This method is not allowed");
}

json depdesc::toJson() const {
    json dep = json::object;
    dep["name"] = name.tx;
    dep["from"] = get<1>(from.extractContent());
    dep["alias"] = alias.tx;
    auto d = doc;
    d.flags = SpaceEngine::HideMain(d.flags);
    dep["doc"] = d.toJson();
    return dep;
}

$depdesc depdesc::fromJson( const json& object, srcdesc space ) {
    $depdesc desc = new depdesc;
    if( !object.is(json::object) ) return nullptr;
    
    if( object.count("name",json::string) ) desc->name = (string)object["name"];
    else return nullptr;

    if( object.count("doc",json::object) ) desc->doc = srcdesc::fromJson(object["doc"]);
    else return nullptr;
    desc->doc.flags |= space.flags;
    desc->doc.package = space.package;

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

Uri depdesc::getDocUri() {
    return getCompilerContext().getSpaceEngine().getUri(doc);
}

bool fragment::is( type t ) const {
    return t == FRAGMENT;
}

$node fragment::clone( $scope scope ) const {
    throw logic_error("fragment::clone():This method is not allowed");
}

Uri fragment::getDocUri() {
    return getCompilerContext().getSpaceEngine().getUri(doc);
}

$fragment fragment::getFragment() {
    return this;
}

CompilerContext& fragment::getCompilerContext() {
    return *context;
}

bool eprototype::is( type t ) const {
    return t == ELEPROTO;
}

$node eprototype::clone( $scope scope ) const {
    clone_node(eprototype,scope);
    ret->etype = etype;
    ret->cons = cons;
    ret->dtype = dtype->clone(scope);
    return ($node)ret;
}

bool aliasdef::is( type t ) const {
    return t == ALIASDEF or t == DEFINITION;
}

$node aliasdef::clone( $scope scope ) const {
    clone_definition(aliasdef,scope);
    if( target ) ret->target = target->clone(scope);
    return ($node)ret;
}

$eprototype eprototype::make( $scope scope, token phrase,$typeexpr dtype, type_t etype, token cons) {
    $eprototype ret = new eprototype;
    ret->phrase = phrase;
    ret->setScope(scope);
    ret->etype = etype;
    ret->dtype = dtype;
    ret->cons = cons;
    return ret;
}

bool classdef::is( type t ) const {
    return t == CLASSDEF or t == DEFINITION;
}

$node classdef::clone( $scope scope ) const {
    clone_definition(classdef,scope);
    ret->abstract = abstract;
    ret->targf = targf;
    ret->preds = preds;
    for( auto targ : targs ) ret->targs << ($eprototype)targ->clone(targ->getScope());
    for( auto super : supers ) ret->supers << ($nameexpr)super->clone(ret);
    for( auto def : defs ) ret->defs << ($definition)def->clone(ret);
    return ($node)ret;
}

bool enumdef::is( type t ) const {
    return t == ENUMDEF or t == DEFINITION;
}

$node enumdef::clone( $scope scope ) const {
    clone_definition(enumdef,scope);
    ret->items = items;
    return ($node)ret;
}

bool metdef::is( type t ) const {
    return t == METHODDEF or t == DEFINITION;
}

$node metdef::clone( $scope scope ) const {
    clone_definition(metdef,scope);
    clone_metprototype(scope);
    ret->raw = raw;
    return ($node)ret;
}

bool opdef::is( type t ) const {
    return t == OPERATORDEF or t == DEFINITION;
}

$node opdef::clone( $scope scope ) const {
    clone_definition(opdef,scope);
    clone_opprototype(scope);
    return ($node)ret;
}

bool attrdef::is( type t ) const {
    return t == ATTRIBUTEDEF or t == DEFINITION;
}

$node attrdef::clone( $scope scope ) const {
    clone_definition(attrdef,scope);
    if( proto ) ret->proto = proto->clone(scope);
    ret->arr = arr;
    ret->meta = meta;
    return ($node)ret;
}

bool opimpl::is( type t ) const {
    return t == OPERATORIMPL or t == IMPLEMENTATION;
}

$node opimpl::clone( $scope scope ) const {
    clone_implementation(opimpl,scope);
    clone_opprototype(scope);
    ret->subtitle = subtitle;
    for( auto super : supers ) ret->supers << ($exprstmt)super->clone(ret);
    for( auto init : initiate ) ret->initiate << ($exprstmt)init->clone(ret);
    return ($node)ret;
}

bool metimpl::is( type t ) const {
    return t == METHODIMPL or t == IMPLEMENTATION;
}

$node metimpl::clone( $scope scope ) const {
    clone_implementation(metimpl,scope);
    clone_metprototype(scope);
    return ($node)ret;
}

bool blockstmt::is( type t ) const {
    return t == BLOCKSTMT or t == STATEMENT;
}

$node blockstmt::clone( $scope scope ) const {
    clone_statement(blockstmt,scope);
    for( auto stmt : *this ) *ret << ($statement)stmt->clone(ret);
    return ($node)ret;
}

bool element::is( type t ) const {
    return t == ELEMENT or t == STATEMENT;
}

$node element::clone( $scope scope ) const {
    clone_statement(element,scope);
    if( proto ) ret->proto = proto->clone(scope);
    if( init ) ret->init = init->clone(scope);
    ret->array = array;
    return ($node)ret;
}

bool fctrlstmt::is( type t ) const {
    return t == FCTRLSTMT or t == STATEMENT;
}

$node fctrlstmt::clone( $scope scope ) const {
    clone_statement(fctrlstmt,scope);
    ret->action = action;
    if( expr ) ret->expr = expr->clone(scope);
    return ($node)ret;
}

bool branchstmt::is( type t ) const {
    return t == BRANCHSTMT or t == STATEMENT;
}

$node branchstmt::clone( $scope scope ) const {
    clone_statement(branchstmt,scope);
    if( condition ) ret->condition = condition->clone(scope);
    if( branch_true ) ret->branch_true = branch_true->clone(scope);
    if( branch_false ) ret->branch_false = branch_false->clone(scope);
    return ($node)ret;
}

bool switchstmt::is( type t ) const {
    return t == SWITCHSTMT or t == STATEMENT;
}

$node switchstmt::clone( $scope scope ) const {
    throw logic_error("switchstmt::clone(): method not ready yet");
}

bool loopstmt::is( type t ) const {
    return t == LOOPSTMT or t == STATEMENT;
}

$node loopstmt::clone( $scope scope ) const {
    clone_statement(loopstmt,scope);
    ret->ltype = ltype;
    if( con ) ret->con = con->clone(scope);
    if( it ) ret->it = it->clone(scope);
    if( key ) ret->key = key->clone(ret);
    if( ctrl ) ret->ctrl = ctrl->clone(ret);
    if( body ) ret->body = body->clone(ret);
    return ($node)ret;
}

bool assumestmt::is( type t ) const {
    return t == ASSUMESTMT or t == STATEMENT;
}

$node assumestmt::clone( $scope scope ) const {
    clone_statement(assumestmt,scope);
    if( expr ) ret->expr = expr->clone(scope);
    if( variable ) ret->variable = variable->clone(scope);
    if( branch_true ) ret->branch_true = branch_true->clone(ret);
    if( branch_false ) ret->branch_false = branch_false->clone(scope);
    return ($node)ret;
}

bool dostmt::is( type t ) const {
    return t == DOSTMT or t == STATEMENT;
}

$node dostmt::clone( $scope scope ) const {
    clone_statement(dostmt,scope);
    ret->when = when;
    if( task ) ret->task = task->clone(scope);
    if( on ) ret->on = on->clone(scope);
    if( then ) ret->then = then->clone(scope);
    return ($node)ret;
}

// bool trystmt::is( type t ) const {
//     return t == TRYSTMT or t == STATEMENT;
// }

// bool catchstmt::is( type t ) const {
//     return t == CATCHSTMT or t == STATEMENT;
// }

// bool throwstmt::is( type t ) const {
//     return t == THROWSTMT or t == STATEMENT;
// }

bool constant::is( type t ) const {
    return t == CONSTANT or t == EXPRESSION or t == STATEMENT;
}

$node constant::clone( $scope scope ) const {
    clone_expression(constant,scope);
    ret->value = value;
    return ($node)ret;
}

bool nameexpr::is( type t ) const {
    return t == NAMEEXPR or t == EXPRESSION or t == STATEMENT;
}

$nameexpr nameexpr::make( $scope scope, token phrase, token name ) {
    auto ret = new nameexpr;
    ret->phrase = phrase;
    ret->setScope(scope);
    ret->etype = name_expr;
    ret->name = name;
    return ret;
}

$node nameexpr::clone( $scope scope ) const {
    clone_expression(nameexpr,scope);
    for( auto targ : targs ) ret->targs << ($eprototype)targ->clone(scope);
    if( next ) ret->next = next->clone(scope);
    return ($node)ret;
}

bool typeexpr::is( type t ) const {
    return t == TYPEEXPR or t == EXPRESSION or t == STATEMENT;
}

$typeexpr typeexpr::make( $scope scope, token phrase, typeid_t id, anything sub ) {
    $typeexpr type = new typeexpr;
    type->phrase = phrase;
    type->setScope(scope);
    type->etype = type_expr;
    type->id = id;
    type->sub = sub;
    return type;
}

$node typeexpr::clone( $scope scope ) const {
    clone_expression(typeexpr,scope);
    ret->id = id;
    if( auto nsub = ($node)sub; sub ) {
        ret->sub = nsub->clone(scope);
    } else if( auto call = ($callable_type)sub; call ) {
        $callable_type nc = new callable_type;
        nc->phrase = call->phrase;
        if( call->ret_proto ) nc->ret_proto = call->ret_proto->clone(scope);
        for( auto arg : call->arg_protos ) nc->arg_protos << ($eprototype)arg->clone(scope);
        nc->va_arg = call->va_arg;
        ret->sub = nc;
    }
    return ($node)ret;
}

bool typeexpr::is_type( typeid_t t ) const {
    return (id & t) == t;
}

bool monoexpr::is( type t ) const {
    return t == MONOEXPR or t == EXPRESSION or t == STATEMENT;
}

$node monoexpr::clone( $scope scope ) const {
    clone_expression(monoexpr,scope);
    if( operand ) ret->operand = operand->clone(scope);
    return ($node)ret;
}

bool binexpr::is( type t ) const {
    return t == BINEXPR or t == EXPRESSION or t == STATEMENT;
}

$node binexpr::clone( $scope scope ) const {
    clone_expression(binexpr,scope);
    if( left ) ret->left = left->clone(scope);
    if( right ) ret->right = right->clone(scope);
    return ($node)ret;
}

bool callexpr::is( type t ) const {
    return t == CALLEXPR or t == EXPRESSION or t == STATEMENT;
}

$node callexpr::clone( $scope scope ) const {
    clone_expression(callexpr,scope);
    if( callee ) ret->callee = callee->clone(scope);
    for( auto param : params ) ret->params << ($exprstmt)param->clone(scope);
    if( rproto ) ret->rproto = rproto->clone(scope);
    return ($node)ret;
}

bool lambdaexpr::is( type t ) const {
    return t == LAMBDAEXPR or t == EXPRESSION or t == STATEMENT;
}

$node lambdaexpr::clone( $scope scope ) const {
    clone_expression(lambdaexpr,scope);
    clone_callable(scope);
    if( body ) ret->body = body->clone(ret);
    return ($node)ret;
}

bool sctorexpr::is( type t ) const {
    return t == SCTOREXPR or t == EXPRESSION or t == STATEMENT;
}

$node sctorexpr::clone( $scope scope ) const {
    clone_expression(sctorexpr,scope);
    for( auto expr : *this ) *ret << ($exprstmt)expr->clone(scope);
    if( type_indicator ) ret->type_indicator = type_indicator->clone(scope);
    return ($node)ret;
}

bool lctorexpr::is( type t ) const {
    return t == LCTOREXPR or t == EXPRESSION or t == STATEMENT;
}

$node lctorexpr::clone( $scope scope ) const {
    clone_expression(lctorexpr,scope);
    for( auto expr : *this ) *ret << ($exprstmt)expr->clone(scope);
    if( type_indicator ) ret->type_indicator = type_indicator->clone(scope);
    return ($node)ret;
}

bool tctorexpr::is( type t ) const {
    return t == TCTOREXPR or t == EXPRESSION or t == STATEMENT;
}

$node tctorexpr::clone( $scope scope ) const {
    clone_expression(tctorexpr,scope);
    for( auto expr : *this ) *ret << ($exprstmt)expr->clone(scope);
    return ($node)ret;
}

bool newexpr::is( type t ) const {
    return t == NEWEXPR or t == EXPRESSION or t == STATEMENT;
}

$node newexpr::clone( $scope scope ) const {
    clone_expression(newexpr,scope);
    if( ntype ) ret->ntype = ntype->clone(scope);
    if( array ) ret->array = array->clone(scope);
    if( init ) ret->init = init->clone(scope);
    return ($node)ret;
}

bool delexpr::is( type t ) const {
    return t == DELEXPR or t == EXPRESSION or t == STATEMENT;
}

$node delexpr::clone( $scope scope ) const {
    clone_expression(delexpr,scope);
    if( target ) ret->target = target->clone(scope);
    ret->array = array;
    return ($node)ret;
}

bool doexpr::is( type t ) const {
    return t == DOEXPR or t == EXPRESSION or t == STATEMENT;
}

$node doexpr::clone( $scope scope ) const {
    clone_expression(doexpr,scope);
    if( task ) ret->task = task->clone(scope);
    return ($node)ret;
}

bool tconvexpr::is( type t ) const {
    return t == TCONVEXPR or t == EXPRESSION or t == STATEMENT;
}

$node tconvexpr::clone( $scope scope ) const {
    clone_expression(tconvexpr,scope);
    if( org ) ret->org = org->clone(scope);
    if( proto ) ret->proto = proto->clone(scope);
    return ($node)ret;
}

bool aspectexpr::is( type t ) const {
    return t == ASPECTEXPR or t == EXPRESSION or t == STATEMENT;
}

$node aspectexpr::clone( $scope scope ) const {
    clone_expression(aspectexpr,scope);
    if( host ) ret->host = host->clone(scope);
    ret->title = title;
    return ($node)ret;
}

bool mbrexpr::is( type t ) const {
    return t == MBREXPR or t == EXPRESSION or t == STATEMENT;
}

$node mbrexpr::clone( $scope scope ) const {
    clone_expression(mbrexpr,scope);
    if( host ) ret->host = host->clone(scope);
    if( nav ) ret->nav = nav->clone(scope);
    return ($node)ret;
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

$callable_type callable::getType() const {
    auto call = new callable_type;
    call->ret_proto = ret_proto;
    for( auto argument : arguments )
        call->arg_protos << argument->proto;
    call->va_arg = va_arg;
    return call;
}

SyntaxContext::wb SyntaxContext::enter() {
    ws << states.size();
    return std::move(wb(states.size(),ws.size(),*this));
}

bool SyntaxContext::working() {
    if( auto sub = states.size() - ws[-1]; sub > 0 ) return true;
    else return movo(sub), ws.pop(), false;
}

SyntaxContext::SyntaxContext( srcdesc dc, tokens& src, Diagnostics& ds ): doc(dc),source(src),diagnostics(ds),it(src.begin()) {
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
                def_padding = constructClassDefinition(frag);
                if( !def_padding ) return nullptr;
            } else if( it->is(VT::ENUM) ) {
                def_padding = constructEnumerateDefinition(frag);
                if( !def_padding ) return nullptr;
            } else if( it->is(VT::OPERATOR) ) {
                impl_padding = constructOperatorImplementation(frag);
                if( !impl_padding ) return nullptr;
            } else if( it->is(VT::METHOD) ) {
                impl_padding = constructMethodImplementation(frag);
                if( !impl_padding ) return nullptr;
            } else if( it->is(VN::CLASSDEF,VN::ALIASDEF,VN::ENUMDEF) ) {
                frag->defs << def_padding;
                stay();
            } else if( it->is(VN::METIMPL,VN::OPIMPL) ) {
                frag->impls << impl_padding;
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
                sig->entry.doc = doc;
                sig->entry.mark = *it;
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
            else if( it->is(VT::O::AS) ) movi(5);
            else if( it->is(VN::LIST) ) stay();
            else redu(-2,VN::DEPENDENCY);
            break;
        case 3:
            if( it->is(VT::L::LABEL,VT::O::MEMBER,VT::L::STRING,VT::L::CHAR) ) {
                ref->from = *it;
                if( auto [success,package,diag] = ref->from.extractContent(); success ) {
                    if( !package.empty() and package != "alioth" and package != "." ) {
                        auto locator = PackageLocator::Parse(package);
                        if( !locator ) {
                            if( diagnostic ) diagnostics("73", *it, locator.toString() );
                            return nullptr;
                        }
                    }
                } else {
                    if( diagnostic ) {
                        diagnostics += diag;
                        diagnostics("72", *it );
                    }
                    return nullptr;
                }
                movi(4);
            } else {
                if( diagnostic ) diagnostics("25",*it);
                return nullptr;
            }
            break;
        case 4:
            if( it->is(VT::O::AS) ) {redu(-2,VN::LIST);}
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

$eprototype SyntaxContext::constructElementPrototype( $scope scope, bool not_care ) {
    $eprototype ref = new eprototype;
    ref->setScope(scope);
    bool et_skip = false; // 是否跳过了元素类型

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
                et_skip = true;
                ref->etype = eprototype::var;
                movi(2,0);
            } break;
        case 2:
            if( it->is(VN::TYPEEXPR) ) {
                if( not not_care and et_skip and ref->dtype->is_type(UnknownType) )
                    return diagnostics("43", *it), nullptr;
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
                ref->name = *it;
                movi(3);
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
                ref->target = nm;
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
                if( auto def = constructEnumerateDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(VT::METHOD) ) {
                if( auto def = constructMethodDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(VT::OPERATOR, VT::DELETE) ) {
                if( auto def = constructOperatorDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(CT::ELETYPE,VT::VAR) ) {
                if( auto def = constructAttributeDefinition(ref); def ) padding = def;
                else return nullptr;
            } else if( it->is(VN::ALIASDEF,VN::CLASSDEF,VN::ENUMDEF,VN::OPDEF,VN::METDEF,VN::ATTRDEF) ) {
                padding->premise = premise;
                if( !branch ) premise.clear();
                ref->defs << padding;
                stay();
            } else if( it->is(VT::L::I::N) ) {
                auto pi = stol(it->tx);
                if( pi >= ref->preds.size() )
                    return diagnostics("79", *it), nullptr;
                premise.insert(pi);
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
                auto pi = stol(it->tx);
                if( pi >= ref->preds.size() )
                    return diagnostics("79", *it), nullptr;
                premise.insert(pi);
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
                if( auto n = constructNameExpression(scope,true); n ) {
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
                bool found = false;
                for( auto targ : ref->targf ) if( *it == targ ) {found = true; break;}
                if( !found ) return diagnostics("106", *it), nullptr;
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
            if( it->is(VT::O::AND) ) {
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
                    if( t->name == it->tx ) {
                        diagnostics("44", *it);
                        diagnostics[-1]( diagnostics.prefix(), "45", *it);
                        return nullptr;
                    }
                }
                $constant c = new constant();
                c->phrase = *it;
                c->setScope(ref);
                c->etype = exprstmt::constant;
                c->name = c->value = *it;
                ref->items << c;
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

$attrdef SyntaxContext::constructAttributeDefinition( $scope scope ) {
    $attrdef ref = new attrdef;
    ref->setScope(scope);
    ref->proto = new eprototype;
    ref->proto->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::ELETYPE,VT::VAR) ) {
                switch( it->id ) {
                    case VT::VAR: ref->proto->etype = eprototype::var; break;
                    case VT::OBJ: ref->proto->etype = eprototype::obj; break;
                    case VT::PTR: ref->proto->etype = eprototype::ptr; break;
                    case VT::REF: ref->proto->etype = eprototype::ref; break;
                    case VT::REL: ref->proto->etype = eprototype::rel; break;
                    default: return internal_error, nullptr;
                }
                movi(2);
            } else {
                return diagnostics("38", *it), nullptr;
            } break;
        case 2:
            if( it->is(PVT::PUBLIC, PVT::PROTECTED, PVT::PRIVATE) ) {
                if( ref->visibility ) return diagnostics("27", *it), nullptr;
                else ref->visibility = *it;
                stay();
            } else if( it->is(VT::META) ) {
                if( ref->meta ) return diagnostics("27", *it), nullptr;
                else ref->meta = *it;
                stay();
            } else if( it->is(VT::CONST) ) {
                if( ref->proto->cons ) return diagnostics("27", *it), nullptr;
                else ref->proto->cons = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return diagnostics("28", *it), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::O::L) ) {
                movi(5);
            } else if( it->is(VN::ITEM) ) {
                stay();
            } else {
                movi(4, 0);
            } break;
        case 4:
            if( it->is(VN::TYPEEXPR) ) {
                redu(4, VN::ATTRDEF);
            } else if( auto t = constructTypeExpression(scope, true); t ) {
                if( t->is_type(UnknownType) )
                    return diagnostics("31", *it), nullptr;
                ref->proto->dtype = t;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::L::I::N) ) {
                ref->arr << stol(it->tx);
                movi(6);
            } else {
                return diagnostics("51", *it), nullptr;
            } break;
        case 6:
            if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::ITEM);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->proto->phrase = *it;
    ref->phrase = *it;
    return ref;
}

$opdef SyntaxContext::constructOperatorDefinition( $scope scope ) {
    $opdef ref = new opdef;
    ref->setScope(scope);
    $eprototype cur_arg = nullptr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::OPERATOR, VN::MORE) ) {
                movi(2);
            } else if( it->is(VT::DELETE) ) {
                if( ref->modifier ) return diagnostics("27", *it), nullptr;
                ref->modifier = *it;
                movi(5);
            } else {
                return diagnostics("21", VT::OPERATOR, *it), nullptr;
            } break;
        case 2:
            if( it->is(PVT::ISM,PVT::REV,PVT::PREFIX,PVT::SUFFIX,VT::DEFAULT) ) {
                if( ref->modifier ) return diagnostics("27", *it), nullptr;
                ref->modifier = *it;
                stay();
            } else if( it->is(VT::CONST) ) {
                if( ref->cons ) return diagnostics("27", *it), nullptr;
                ref->cons = *it;
                stay();
            } else if( constructOperatorLabel(scope, ref->subtitle, ref->ret_proto ) and it->is(CT::OP_LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VN::LIST) ) {
                movi(4);
            } else if( !ref->modifier.is(VT::DEFAULT,VT::DELETE) ) {
                if( !ref->name.is(PVT::SCTOR) ) {
                    if( !constructParameterList(scope, *ref, false) ) return nullptr;
                } else {
                    if( !constructParameterList(scope, *ref, true ) ) return nullptr;
                } 
            } else {
                movi(4,0);
            } break;
        case 4:
            if( it->is(VN::ELEPROTO) ) {
                redu(4, VN::OPDEF);
            } else if( ref->name.is(PVT::SCTOR,PVT::LCTOR,PVT::DTOR,PVT::CCTOR,PVT::MCTOR,PVT::AS,PVT::MOVE) ) {
                redu(-4, VN::OPDEF);
            } else if( auto proto = constructElementPrototype(scope, true); proto ){
                ref->ret_proto = proto;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::DEFAULT) ) {
                movi(6);
            } else {
                return diagnostics("21", VT::DEFAULT, *it), nullptr;
            } break;
        case 6:
            if( it->is(VT::OPERATOR) ) {
                redu(2, VN::MORE);
            } else {
                return diagnostics("21", VT::OPERATOR, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$metdef SyntaxContext::constructMethodDefinition( $scope scope ) {
    $metdef ref = new metdef;
    ref->setScope(scope);
    $eprototype cur_arg = nullptr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::METHOD) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::METHOD, *it), nullptr;
            } break;
        case 2:
            if( it->is(PVT::PRIVATE, PVT::PUBLIC, PVT::PROTECTED) ) {
                if( ref->visibility ) return diagnostics("27", *it), nullptr;
                else ref->visibility = *it;
                stay();
            } else if( it->is(VT::META) ) {
                if( ref->meta or ref->cons ) return diagnostics("27", *it), nullptr;
                else ref->meta = *it;
                stay();
            } else if( it->is(VT::CONST) ) {
                if( ref->cons or ref->meta ) return diagnostics("27", *it), nullptr;
                else ref->cons = *it;
                stay();
            } else if( it->is(PVT::RAW) ) {
                if( ref->raw ) return diagnostics("27", *it), nullptr;
                ref->raw = *it;
                movi(5);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else if( it->is(PVT::ATOMIC,PVT::ASYNC) ) {
                if( ref->mode ) return diagnostics("27", *it), nullptr;
                else ref->mode = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return diagnostics("28", *it), nullptr;
            } break;
        case 3:
            if( it->is(VN::LIST) ) {
                movi(4);
            } else if( constructParameterList(scope, *ref, true ) ) {
                bool set = false;
                for( auto& arg : ref->arguments ) {
                    if( arg->init ) set = true;
                    else if( set ) return diagnostics("53", arg->phrase), nullptr;
                }
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::ELEPROTO) ) {
                redu(4, VN::METDEF);
            } else if( auto p = constructElementPrototype(scope, true); p ) {
                ref->ret_proto = p;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::L::STRING) ) {
                ref->raw = *it;
                redu(1, VN::MORE);
            } else {
                redu(-1, VN::MORE);
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$opimpl SyntaxContext::constructOperatorImplementation( $scope scope ) {
    $opimpl ref = new opimpl;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::OPERATOR) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::OPERATOR), nullptr;
            } break;
        case 2:
            if( it->is(VT::L::LABEL) ) {
                ref->host = constructNameExpression(scope,true);
                if( !ref->host ) return nullptr;
            } else if( it->is(VN::NAMEEXPR) ) {
                movi(3);
            } else {
                return diagnostics("28", *it), nullptr;
            } break;
        case 3:
            if( it->is(PVT::ISM,PVT::REV,PVT::PREFIX,PVT::SUFFIX) ) {
                if( ref->modifier ) return diagnostics("27", *it), nullptr;
                ref->modifier = *it;
                stay();
            } else if( it->is(VT::CONST) ) {
                if( ref->cons ) return diagnostics("27", *it), nullptr;
                ref->cons = *it;
                stay();
            } else if( constructOperatorLabel(ref, ref->subtitle, ref->ret_proto ) and it->is(CT::OP_LABEL) ) {
                ref->name = *it;
                movi(4);
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::LIST) ) {
                movi(5);
            } else {
                if( !constructParameterList(ref, *ref, false) ) return nullptr;
            } break;
        case 5:
            if( it->is(VN::ELEPROTO) ) {
                movi(6);
            } else if( it->is(VN::ELEPROTO) or ref->name.is(PVT::SCTOR,PVT::CCTOR,PVT::MCTOR,PVT::LCTOR) ) {
                movi(6, 0);
            } else if( auto proto = constructElementPrototype(ref,true); proto ) {
                ref->ret_proto = proto;
            } else {
                return nullptr;
            } break;
        case 6:
            if( it->is(VN::BLOCKSTMT) ) {
                redu(6, VN::OPIMPL);
                ref->body->phrase = *it;
            } else if( ref->name.is(PVT::SCTOR,PVT::CCTOR,PVT::MCTOR,PVT::LCTOR) ) {
                ref->body = new blockstmt;
                ref->body->setScope(ref);
                movi(7,0);
            } else if( auto stmt = constructBlockStatement(ref); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 7:
            if( it->is(VT::O::SC::O::S) ) {
                movi(8);
            } else {
                return diagnostics("21", VT::O::SC::O::S, *it), nullptr;
            } break;
        case 8:
            if( it->is(VT::O::SC::C::S) ) {
                redu(2, VN::BLOCKSTMT);
            } else if( it->is(VT::O::SC::O::S) ) {
                auto stmt = constructStructuralConstructingExpression(ref);
                if( !stmt ) return nullptr;
                ref->supers << ($exprstmt)stmt;
            } else if( it->is(VT::O::SC::O::L) ) {
                auto stmt = constructListConstructingExpression(ref);
                if( !stmt ) return nullptr;
                ref->supers << ($exprstmt)stmt;
            } else if( it->is(VT::O::SCOPE) ) {
                movi(9);
            } else if( it->is(VN::EXPRSTMT) ) {
                stay();
            } else if( it->is(VN::FINAL) ) {
                redu(2,VN::BLOCKSTMT);
            } else {
                movi(10,0);
            } break;
        case 9:
            if( it->is(VT::L::LABEL) ) {
                auto name = *it;
                stay();
                if( !it->is(VT::O::SC::COLON) ) return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
                stay();
                auto expr = constructExpressionStatement(ref);
                if( !expr ) return nullptr;
                expr->name = name;
                ref->initiate << expr;
                stay();
            } else if( it->is(VT::O::SC::COMMA) ) {
                stay();
                if( !it->is(VT::L::LABEL) ) return diagnostics("29", *it), nullptr;
            } else if( it->is(VT::O::SC::SEMI) ) {
                movi(10);
            } else if( it->is(VN::FINAL) ) {
                redu(3, VN::BLOCKSTMT);
            } else {
                return diagnostics("21", VT::O::SC::C::S, *it), nullptr;
            } break;
        case 10:
            if( it->is( CT::STATEMENT, VT::O::SC::SEMI ) ) {
                stay();
            } else if( it->is(VT::O::SC::C::S) ) {
                redu(1, VN::FINAL);
            } else if( auto stmt = constructStatement(ref->body, false, true); stmt ) {
                *ref->body << stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$metimpl SyntaxContext::constructMethodImplementation( $scope scope ) {
    $metimpl ref = new metimpl;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::METHOD) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::METHOD), nullptr;
            } break;
        case 2:
            if( it->is(VT::META) ) {
                if( ref->meta ) return diagnostics("27", *it), nullptr;
                ref->meta = *it;
                stay();
            } else if( it->is(PVT::ATOMIC,PVT::ASYNC) ) {
                if( ref->mode ) return diagnostics("27", *it), nullptr;
                ref->mode = *it;
                stay();
            } else if( it->is(VT::CONST) ) {
                if( ref->cons ) return diagnostics("27", *it), nullptr;
                ref->cons = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                auto name = constructNameExpression(scope, true);
                $nameexpr last = nullptr;
                if( !name ) return nullptr;
                if( !name->next )
                    diagnostics("54", name->phrase);
                for( last = name; last->next != nullptr and last->next->next != nullptr; last = last->next );
                ref->host = last->next;
                last->next = nullptr;
                last = ref->host?ref->host:name;
                ref->host = name;
                if( last->targs.size() ) return diagnostics("55", last->phrase), nullptr;
                ref->name = last->name;
            } else if( it->is(VN::NAMEEXPR) ) {
                movi(3);
            } else {
                return diagnostics("28", *it), nullptr;
            } break;
        case 3:
            if( it->is(VN::LIST) ) {
                movi(4);
            } else if( constructParameterList(ref, *ref, false) ) {
                /** NOTHING TO BE DONE */
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::ELEPROTO) ) {
                movi(5);
            } else if( auto proto = constructElementPrototype(ref, true); proto ) {
                ref->ret_proto = proto;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VN::BLOCKSTMT) ) {
                redu(5, VN::METIMPL);
            } else if( auto block = constructBlockStatement(ref); block ) {
                ref->body = block;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$blockstmt SyntaxContext::constructBlockStatement( $scope scope ) {
    $blockstmt ref = new blockstmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::SC::O::S) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::O::SC::O::S, *it), nullptr;
            } break;
        case 2:
            if( it->is(CT::STATEMENT,VT::O::SC::SEMI) ) {
                stay();
            } else if( it->is(VT::O::SC::C::S) ) {
                redu(2, VN::BLOCKSTMT);
            } else if( auto stmt = constructStatement(ref, false, true); stmt ) {
                *ref << stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$element SyntaxContext::constructElementStatement( $scope scope, bool autowire ) {
    $element ref = new element;
    ref->setScope(scope);
    ref->proto = new eprototype;
    ref->proto->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::ELETYPE,VT::VAR) ) {
                switch( it->id ) {
                    case VT::VAR: ref->proto->etype = eprototype::var; break;
                    case VT::OBJ: ref->proto->etype = eprototype::obj; break;
                    case VT::PTR: ref->proto->etype = eprototype::ptr; break;
                    case VT::REF: ref->proto->etype = eprototype::ref; break;
                    case VT::REL: ref->proto->etype = eprototype::rel; break;
                }
                movi(2);
            } else if( it->is(VT::L::LABEL,VT::CONST) ) {
                ref->proto->etype = eprototype::var;
                movi(2,0);
            } else {
                return diagnostics("38", *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::CONST) ) {
                if( ref->proto->cons ) return diagnostics("27", *it), nullptr;
                ref->proto->cons = *it;
                stay();
            } else if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(3);
            } else {
                return diagnostics("58", *it), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::ASSIGN) ) {
                if( autowire ) return diagnostics("21", VT::O::SC::COLON, *it ), nullptr;
                if( !ref->proto->dtype ) ref->proto->dtype = typeexpr::make(scope, *it);
                movi(4);
            } else if( it->is(VT::O::SC::SEMI) ) {
                if( autowire ) return diagnostics("21", VT::O::SC::COLON, *it ), nullptr;
                if( !ref->proto->dtype ) ref->proto->dtype = typeexpr::make(scope, *it);
                redu(-3, VN::ELEMENTSTMT);
            } else if( autowire and it->is(PVT::ON,VT::O::POINTER) ) {
                ref->proto->dtype = typeexpr::make(scope, *it);
                redu(-3, VN::ELEMENTSTMT);
            } else if( autowire ) {
                return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
            } else if( it->is(VN::TYPEEXPR,VN::MORE) ) {
                stay();
            } else if( it->is(VT::O::SC::O::L) ) {
                if( ref->proto->dtype ) redu(-3, VN::ELEMENTSTMT);
                if( autowire ) return diagnostics("70", *it), nullptr;
                movi(5);
            } else if( ref->proto->dtype ) {
                redu(-3, VN::ELEMENTSTMT);
            } else if( auto type = constructTypeExpression(scope,true); type ) {
                ref->proto->dtype = type;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::EXPRSTMT) ) {
                if( autowire ) return diagnostics("69", *it), nullptr;
                redu(4, VN::ELEMENTSTMT);
            } else if( ref->array.size() ) {
                auto expr = constructListConstructingExpression(scope);
                if( !expr ) return nullptr;
                ref->init = expr;
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->init = expr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::L::I::N) ) {
                ref->array << stol(*it);
                movi(6);
            } else if( it->is(VT::O::SC::C::L) ) {
                ref->array << -1;
                redu(1, VN::MORE);
            } else {
                return diagnostics("51", *it), nullptr;
            } break;
        case 6:
            if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::MORE);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->proto->phrase = *it;
    ref->phrase = *it;
    return ref;
}

$exprstmt SyntaxContext::constructExpressionStatement( $scope scope, bool intuple ) {
    $exprstmt ref;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::CONSTANT) ) {
                ref = constructConstantExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(CT::PREFIX) ) {
                ref = new monoexpr;
                switch( it->id ) {
                    case VT::O::MINUS: ref->etype = exprstmt::negative; break;
                    case VT::O::NOT: ref->etype = exprstmt::lnot; break;
                    case VT::O::B::REV: ref->etype = exprstmt::brev; break;
                    case VT::O::B::AND: ref->etype = exprstmt::address; break;
                    case VT::O::MUL: ref->etype = exprstmt::refer; break;
                    case VT::O::INCREMENT: ref->etype = exprstmt::preinc; break;
                    case VT::O::DECREMENT: ref->etype = exprstmt::predec; break;
                    default: return internal_error, nullptr;
                }
                movi(4);
            } else if( it->is(VT::L::LABEL,VT::CLASS) ) {
                ref = constructNameExpression(scope, false);
                if( !ref ) return nullptr;
            } else if( it->is(VT::O::CONV) ) {
                ref = constructLambdaExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::O::SC::O::A) ) {
                movi(2);
            } else if( it->is(VT::O::SC::O::L) ) {
                ref = constructListConstructingExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::O::SC::O::S) ) {
                ref = constructStructuralConstructingExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::O::LT) ) {
                ref = constructTupleConstructingExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::NEW) ) {
                ref = constructNewExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::DELETE) ) {
                ref = constructDeleteExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VT::DO) ) {
                ref = constructDoExpression(scope);
                if( !ref ) return nullptr;
            } else if( it->is(VN::EXPRSTMT,VN::NAMEEXPR,VN::TYPEEXPR) ) {
                movi(5);
            } else {
                return diagnostics("65", *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::EXPRSTMT) ) {
                movi(3);
            } else {
                ref = constructExpressionStatement(scope);
                if( !ref ) return nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::C::A) ) {
                redu(2, VN::EXPRSTMT);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } break;
        case 4:
            if( it->is(VN::EXPRSTMT) ) {
                redu(1, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                (($monoexpr)ref)->operand = expr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(CT::SUFFIX) ) {
                $monoexpr nr = new monoexpr;
                switch( it->id ) {
                    case VT::O::INCREMENT: nr->etype = exprstmt::postinc; break;
                    case VT::O::DECREMENT: nr->etype = exprstmt::postdec; break;
                    default: return internal_error, nullptr;
                }
                nr->operand = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                redu(1, VN::EXPRSTMT);
            } else if( it->is(VT::O::SC::O::L) ) {
                $monoexpr nr = new monoexpr;
                nr->etype = exprstmt::index;
                nr->operand = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(6);
            } else if( it->is(VT::O::GT) and intuple ) {
                redu(-2, VN::EXPRSTMT);
            } else if( it->is(CT::INFIX) ) {
                auto prev = prio(*(it-2));
                if( prev and prev < prio(*it)) redu(-2,VN::EXPRSTMT);
                $binexpr nr = new binexpr;
                switch( it->id ) {
                    case VT::O::SHL: nr->etype = exprstmt::shl; break;
                    case VT::O::SHR: nr->etype = exprstmt::shr; break;
                    case VT::O::B::AND: nr->etype = exprstmt::band; break;
                    case VT::O::B::XOR: nr->etype = exprstmt::bxor; break;
                    case VT::O::B::OR: nr->etype = exprstmt::bor; break;
                    case VT::O::MOL: nr->etype = exprstmt::mol; break;
                    case VT::O::MUL: nr->etype = exprstmt::mul; break;
                    case VT::O::DIV: nr->etype = exprstmt::div; break;
                    case VT::O::PLUS: nr->etype = exprstmt::add; break;
                    case VT::O::MINUS: nr->etype = exprstmt::sub; break;
                    case VT::O::LT: nr->etype = exprstmt::lt; break;
                    case VT::O::LE: nr->etype = exprstmt::le; break;
                    case VT::O::GT: nr->etype = exprstmt::gt; break;
                    case VT::O::GE: nr->etype = exprstmt::ge; break;
                    case VT::O::EQ: nr->etype = exprstmt::eq; break;
                    case VT::O::NE: nr->etype = exprstmt::ne; break;
                    case VT::O::AND: nr->etype = exprstmt::land; break;
                    case VT::O::OR: nr->etype = exprstmt::lor; break;
                    case VT::O::ASSIGN: nr->etype = exprstmt::assign; break;
                    case VT::O::ASS::B::AND: nr->etype = exprstmt::bandass; break;
                    case VT::O::ASS::B::OR: nr->etype = exprstmt::borass; break;
                    case VT::O::ASS::B::XOR: nr->etype = exprstmt::bxorass; break;
                    case VT::O::ASS::SHL: nr->etype = exprstmt::shlass; break;
                    case VT::O::ASS::SHR: nr->etype = exprstmt::shrass; break;
                    case VT::O::ASS::PLUS: nr->etype = exprstmt::addass; break;
                    case VT::O::ASS::MINUS: nr->etype = exprstmt::subass; break;
                    case VT::O::ASS::MUL: nr->etype = exprstmt::mulass; break;
                    case VT::O::ASS::DIV: nr->etype = exprstmt::divass; break;
                    case VT::O::ASS::MOL: nr->etype = exprstmt::molass; break;
                    default: return internal_error, nullptr;
                }
                nr->left = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(8);
            } else if( it->is(VT::O::MEMBER,VT::O::POINTER) ) {
                auto prev = prio(*(it-2));
                if( prev and prev < prio(*it)) redu(-2,VN::EXPRSTMT);
                $mbrexpr nr = new mbrexpr;
                if( it->id == VT::O::MEMBER ) nr->etype = exprstmt::member;
                else nr->etype = exprstmt::pointer;
                nr->host = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(13);
            } else if( it->is(VT::O::SHARP) ) {
                auto prev = prio(*(it-2));
                if( prev and prev < prio(*it)) redu(-2,VN::EXPRSTMT);
                $aspectexpr nr = new aspectexpr;
                nr->etype = exprstmt::aspect;
                nr->host = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(14);
            } else if( it->is(VT::O::SC::O::A) ) {
                $callexpr nr = new callexpr;
                nr->etype = exprstmt::call;
                nr->callee = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(9);
            } else if( it->is(VT::O::AS,VT::O::TREAT) ) {
                auto prev = prio(*(it-2));
                if( prev and prev < prio(*it)) redu(-2,VN::EXPRSTMT);
                $tconvexpr nr = new tconvexpr;
                if( it->id == VT::O::AS ) nr->etype = exprstmt::as;
                else nr->etype = exprstmt::treat;
                nr->org = ref;
                ref->phrase = *(it-1);
                ref->setScope(nullptr);
                ref->setScope(scope);
                ref = nr;
                movi(12);
            } else {
                redu(-2,VN::EXPRSTMT);
            } break;
        case 6:
            if( it->is(VN::EXPRSTMT) ) {
                movi(7);
            } else if( auto id = constructExpressionStatement(scope); id ) {
                (($monoexpr)ref)->operand = id;
            } else {
                return nullptr;
            } break;
        case 7:
            if( it->is(VT::O::SC::C::L) ) {
                redu(3, VN::EXPRSTMT);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        case 8: 
            if( it->is(VN::EXPRSTMT) ) {
                redu(2, VN::EXPRSTMT);
            } else {
                auto& right = (($binexpr)ref)->right = constructExpressionStatement(scope);
                if( !right ) return nullptr;
            } break;
        case 9:
            if( it->is(VT::O::SC::C::A) ) {
                redu(2,VN::EXPRSTMT);
            } else if( it->is(VN::EXPRSTMT) ) {
                movi(10);
            } else if( it->is(VT::O::GENERATE) ) {
                movi(11);
            } else if( it->is(VN::MORE) ) {
                stay();
                auto arg = constructExpressionStatement(scope);
                if( !arg ) return nullptr;
                (($callexpr)ref)->params << arg;
            } else if( it->is(VN::FINAL) ) {
                stay();
                if( !it->is(VT::O::SC::C::A) ) return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } else if( auto arg = constructExpressionStatement(scope); arg ) {
                (($callexpr)ref)->params << arg;
            } else {
                return nullptr;
            } break;
        case 10:
            if( it->is(VT::O::SC::COMMA) ) {
                redu(1, VN::MORE);
            } else if( it->is(VT::O::SC::C::A) ) {
                redu(3, VN::EXPRSTMT);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it),nullptr;
            } break;
        case 11:
            if( it->is(VN::ELEPROTO) ) {
                redu(1, VN::FINAL);
            } else if( auto proto = constructElementPrototype(scope, false); proto ) {
                if( proto->dtype->is_type(UnknownType) ) return diagnostics("31", *it), nullptr;
                (($callexpr)ref)->rproto = proto;
            } else {
                return nullptr;
            } break;
        case 12:
            if( it->is(VN::ELEPROTO) ) {
                redu(2,VN::EXPRSTMT);
            } else if( auto proto = constructElementPrototype(scope, false); proto ) {
                (($tconvexpr)ref)->proto = proto;
            } else {
                return nullptr;
            } break;
        case 13:
            if( it->is(VN::NAMEEXPR) ) {
                redu(2, VN::EXPRSTMT);
            } else if( auto name = constructNameExpression(scope,false); name ) {
                (($mbrexpr)ref)->nav = name;
            } else {
                return nullptr;
            } break;
        case 14:
            if( it->is(VT::L::LABEL,VT::L::I::N) ) {
                (($aspectexpr)ref)->title = *it;
                redu(2, VN::EXPRSTMT);
            } else {
                return diagnostics("66", *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->setScope(scope);
    ref->phrase = *it;
    return ref;
}

$branchstmt SyntaxContext::constructBranchStatement( $scope scope ) {
    $branchstmt ref = new branchstmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::IF) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::IF, *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::O::SC::O::A) ) {
                movi(3);
            } else {
                return diagnostics("21", VT::O::SC::O::A, *it), nullptr;
            } break;
        case 3:
            if( it->is(VN::EXPRSTMT) ) {
                movi(4);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->condition = expr;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VT::O::SC::C::A) ) {
                movi(5);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } break;
        case 5:
            if( it->is(CT::STATEMENT) ) {
                movi(6);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->branch_true = stmt;
            } else {
                return nullptr;
            } break;
        case 6:
            if( it->is(VT::ELSE) ) {
                movi(7);
            } else {
                redu(-6, VN::BRANCHSTMT);
            } break;
        case 7:
            if( it->is(CT::STATEMENT) ) {
                redu(7, VN::BRANCHSTMT);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->branch_false = stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$switchstmt SyntaxContext::constructSwitchStatement( $scope scope ) {
    return not_ready_yet, nullptr;
}

$assumestmt SyntaxContext::constructAssumeStatement( $scope scope ) {
    $assumestmt ref = new assumestmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::ASSUME) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::ASSUME, *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::EXPRSTMT) ) {
                movi(3);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->expr = expr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(PVT::IS) ) {
                movi(4);
            } else {
                return diagnostics("21", "is", *it), nullptr;
            } break;
        case 4:
            if( it->is(VN::ELEPROTO) ) {
                movi(5);
            } else if( auto var = constructElementStatement(scope, false); var ) {
                if( var->proto->dtype->is_type(UnknownType) ) return diagnostics("31", *it), nullptr;
                if( var->init ) return diagnostics("105", var->init->phrase), nullptr;
                ref->variable = var;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(PVT::THEN) ) {
                movi(6);
            } else {
                return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
            } break;
        case 6:
            if( it->is(CT::STATEMENT) ) {
                movi(7);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->branch_true = stmt;
            } else {
                return nullptr;
            } break;
        case 7:
            if( it->is(VT::OTHERWISE) ) {
                movi(8);
            } else {
                redu(-7, VN::ASSUMESTMT);
            } break;
        case 8:
            if( it->is(CT::STATEMENT) ) {
                redu(8, VN::ASSUMESTMT);
            } else if( auto stmt = constructStatement(scope, true, false); stmt ) {
                ref->branch_false = stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$loopstmt SyntaxContext::constructLoopStatement( $scope scope ) {
    $loopstmt ref = new loopstmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::LOOP) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::LOOP, *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::O::SC::COLON) ) {
                ref->ltype = loopstmt::infinite;
                movi(3);
            } else if( it->is(PVT::WHILE) ) {
                ref->ltype = loopstmt::condition;
                movi(4);
            } else if( it->is(PVT::FOR) ) {
                ref->ltype = loopstmt::step;
                movi(8);
            } else if( it->is(VT::DO) ) {
                ref->ltype = loopstmt::suffixed;
                movi(16);
            } else if( it->is(CT::ELETYPE,VT::VAR,VT::CONST,VT::L::LABEL) ) {
                ref->ltype = loopstmt::iterate;
                movi(21,0);
            } else {
                return diagnostics("62", *it), nullptr;
            } break;
        case 3:
            if( it->is(CT::STATEMENT) ) {
                redu(3, VN::LOOPSTMT);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VT::O::SC::O::A) ) {
                movi(5);
            } else {
                return diagnostics("21", VT::O::SC::O::A, *it), nullptr;
            } break;
        case 5:
            if( it->is(VN::EXPRSTMT) ) {
                movi(6);
            } else if( auto expr = constructExpressionStatement(ref); expr ) {
                ref->con = expr;
            } else {
                return nullptr;
            } break;
        case 6:
            if( it->is(VT::O::SC::C::A) ) {
                movi(7);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } break;
        case 7:
            if( it->is(CT::STATEMENT) ) {
                redu(6, VN::LOOPSTMT);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 8:
            if( it->is(VT::O::SC::O::A) ) {
                movi(9);
            } else {
                return diagnostics("21", VT::O::SC::O::A, *it), nullptr;
            } break;
        case 9:
            if( it->is(VN::ELEMENTSTMT,VN::EXPRSTMT) ) {
                movi(10);
            } else if( it->is(VT::O::SC::SEMI) ) {
                movi(10,0);
            } else if( it->is(CT::ELETYPE,VT::VAR,VT::CONST) ) {
                auto ele = constructElementStatement(ref,false);
                if( ele and ele->proto->dtype->is_type(UnknownType) and !ele->init )
                    return diagnostics("31", ele->name), nullptr;
                ref->it = ele;
                if( !ref->it ) return nullptr;
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->it = expr;
            } else {
                return nullptr;
            } break;
        case 10:
            if( it->is(VT::O::SC::SEMI) ) {
                movi(11);
            } else {
                return diagnostics("21", VT::O::SC::SEMI, *it), nullptr;
            } break;
        case 11:
            if( it->is(VN::EXPRSTMT) ) {
                movi(12);
            } else if( it->is(VT::O::SC::SEMI) ) {
                movi(12,0);
            } else if( auto expr = constructExpressionStatement(ref); expr ) {
                ref->con = expr;
            } else {
                return nullptr;
            } break;
        case 12:
            if( it->is(VT::O::SC::SEMI) ) {
                movi(13);
            } else {
                return diagnostics("21", VT::O::SC::SEMI, *it), nullptr;
            } break;
        case 13:
            if( it->is(VN::EXPRSTMT) ) {
                movi(14);
            } else if( it->is(VT::O::SC::C::A) ) {
                movi(14,0);
            } else if( auto expr = constructExpressionStatement(ref); expr ) {
                ref->ctrl = expr;
            } else {
                return nullptr;
            } break;
        case 14:
            if( it->is(VT::O::SC::C::A) ) {
                movi(15);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } break;
        case 15:
            if( it->is(CT::STATEMENT) ) {
                redu(10, VN::LOOPSTMT);
            } else if( auto stmt = constructStatement(ref, true, false); ref ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 16:
            if( it->is(CT::STATEMENT) ) {
                movi(17);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 17:
            if( it->is(PVT::WHILE) ) {
                movi(18);
            } else {
                return diagnostics("21", "while", *it), nullptr;
            } break;
        case 18:
            if( it->is(VT::O::SC::O::A) ) {
                movi(19);
            } else {
                return diagnostics("21", VT::O::SC::O::A, *it), nullptr;
            } break;
        case 19:
            if( it->is(VN::EXPRSTMT) ) {
                movi(20);
            } else if( auto expr = constructExpressionStatement(ref); expr ) {
                ref->con = expr;
            } else {
                return nullptr;
            } break;
        case 20:
            if( it->is(VT::O::SC::C::A) ) {
                redu(7, VN::LOOPSTMT);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            } break;
        case 21:
            if( it->is(VN::ELEMENTSTMT) ) {
                movi(22);
            } else if( auto ele = constructElementStatement(ref,true); ele ) {
                ref->it = ele;
            } else {
                return nullptr;
            } break;
        case 22:
            if( it->is(PVT::ON) ) {
                movi(23);
            } else if( it->is(VT::O::POINTER) ) {
                if( ref->key ) return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
                ref->ltype = loopstmt::keyvalue;
                ref->key = ref->it;
                ref->it = nullptr;
                movi(26);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else {
                return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
            } break;
        case 23:
            if( it->is(VN::EXPRSTMT) ) {
                movi(24);
            } else if( auto expr = constructExpressionStatement(ref); expr ) {
                ref->con = expr;
            } else {
                return nullptr;
            } break;
        case 24:
            if( it->is(VT::DO) ) {
                movi(25);
            } else {
                return diagnostics("21", VT::DO, *it), nullptr;
            } break;
        case 25:
            if( it->is(CT::STATEMENT) ) {
                redu(7, VN::LOOPSTMT);
            } else if( auto stmt = constructStatement(ref, true, false); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        case 26:
            if( it->is(VN::ELEMENTSTMT) ) {
                redu(1, VN::MORE);
            } else if( auto stmt = constructElementStatement(ref, true); stmt ) {
                ref->it = stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$fctrlstmt SyntaxContext::constructFlowControlStatement( $scope scope ) {
    $fctrlstmt ref = new fctrlstmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::BREAK,VT::CONTINUE) ) {
                ref->action = *it;
                movi(2);
            } else if( it->is(VT::RETURN) ) {
                ref->action = *it;
                movi(5);
            } else {
                return internal_error, nullptr;
            } break;
        case 2:
            if( it->is(VT::O::FORCE) ) {
                if( ref->expr or ref->name ) return diagnostics("21", VT::O::SC::SEMI, *it), nullptr;
                ref->action = *it;
                redu(2, VN::FCTRLSTMT);
            } else if( it->is(VT::O::AT) ) {
                if( ref->name ) return diagnostics("21", VT::O::SC::SEMI, *it), nullptr;
                movi(3);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else if( it->is(PVT::AFTER) ) {
                if( ref->expr ) return diagnostics("21", VT::O::SC::SEMI, *it), nullptr;
                movi(4);
            } else {
                redu(-2, VN::FCTRLSTMT);
            } break;
        case 3:
            if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                redu(1, VN::MORE);
            } else {
                return diagnostics("63", *it), nullptr;
            } break;
        case 4:
            if( it->is(VN::EXPRSTMT) ) {
                redu(3, VN::FCTRLSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->expr = expr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::O::SC::SEMI) ) {
                redu(-2, VN::FCTRLSTMT);
            } else if( it->is(VN::EXPRSTMT) ) {
                redu(2, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->expr = expr;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$dostmt SyntaxContext::constructDoStatement( $scope scope ) {
    $dostmt ref = new dostmt;
    ref->setScope(scope);

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::DO) ) {
                movi(2);
            } else {
                return internal_error, nullptr;
            } break;
        case 2:
            if( it->is(VT::O::SC::O::S) ) {
                movi(3,0);
            } else if( it->is(VN::EXPRSTMT) ) {
                movi(6);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->task = expr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VN::BLOCKSTMT) ) {
                movi(4);
            } else if( auto stmt = constructBlockStatement(scope); stmt ) {
                ref->task = stmt;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(PVT::WHEN) ) {
                movi(5);
            } else {
                return diagnostics("21", "when", *it), nullptr;
            } break;
        case 5:
            if( it->is(VT::RETURN) ) {
                redu(5, VN::DOSTMT);
            } else {
                return diagnostics("64", *it), nullptr;
            } break;
        case 6:
            if( it->is(PVT::THEN) ) {
                movi(8);
            } else if( it->is(PVT::ON) ) {
                if( ref->on ) return diagnostics("21", "then", *it), nullptr;
                movi(7);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else {
                return diagnostics("21", "then", *it), nullptr;
            } break;
        case 7:
            if( it->is(VN::EXPRSTMT) ) {
                redu(1, VN::MORE);
            } else if( auto on = constructExpressionStatement(scope); on ) {
                ref->on = on;
            } else {
                return nullptr;
            } break;
        case 8:
            if( it->is(VN::EXPRSTMT) ) {
                redu(4, VN::DOSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->then = expr;
            } else {
                return nullptr;
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
    ref->etype = exprstmt::name_expr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::L::LABEL) ) {
                ref->name = *it;
                movi(2);
            } else if( it->is(VT::CLASS) ) {
                if( absorb ) diagnostics("27", *it );
                else absorb = true;
                stay();
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
            } else if( auto t = constructElementPrototype(scope, false); t ) {
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
            } else if( auto t = constructElementPrototype(scope, false); t ) {
                ref->targs << t;
                if( t->dtype->is_type(UnknownType) ) return diagnostics("31", *it ), nullptr;
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
    ref->etype = exprstmt::type_expr;
    agent<callable_type> call;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(CT::BASIC_TYPE) ) {
                switch( it->id ) {
                    case VT::INT8: ref->id = Int8Type; break;
                    case VT::INT16: ref->id = Int16Type; break;
                    case VT::INT32: ref->id = Int32Type; break;
                    case VT::INT64: ref->id = Int64Type; break;
                    case VT::UINT8: ref->id = Uint8Type; break;
                    case VT::UINT16: ref->id = Uint16Type; break;
                    case VT::UINT32: ref->id = Uint32Type; break;
                    case VT::UINT64: ref->id = Uint64Type; break;
                    case VT::FLOAT32: ref->id = Float32Type; break;
                    case VT::FLOAT64: ref->id = Float64Type; break;
                    case VT::BOOL: ref->id = BooleanType; break;
                    case VT::VOID: ref->id = VoidType; break;
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
            } else if( it->is(VT::O::B::XOR) ) {
                ref->id = ConstraintedPointerType;
                movi(2);
            } else if( it->is(VT::L::LABEL) ) {
                ref->id = NamedType;
                movi(3, 0);
            } else if( it->is(VT::L::THIS) ) {
                movi(4);
            } else if( it->is(VT::O::SC::O::A) ) {
                ref->id = CallableType;
                call = ref->sub = new callable_type;
                movi(5);
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
        case 5:
            if( it->is(VT::O::SC::C::A,VT::O::GENERATE) ) {
                movi(7,0);
            } else if( it->is(VT::O::ETC) ) {
                if( call->va_arg ) return diagnostics("21", VT::O::GENERATE, *it), nullptr;
                call->va_arg = *it;
                stay();
            } else if( it->is(VN::ELEPROTO) ) {
                movi(6);
            } else if( it->is(VN::MORE) ) {
                stay();
                if( it->is(VT::O::SC::C::A,VT::O::GENERATE) ) return diagnostics("31", *it), nullptr;
            } else if( it->is(VN::FINAL) ) {
                redu(2, VN::TYPEEXPR);
            } else if( auto p = constructElementPrototype(scope, false); p ) {
                call->arg_protos << p;
            } else {
                return nullptr;
            } break;
        case 6:
            if( it->is(VT::O::SC::COMMA) ) {
                redu(1, VN::MORE);
            } else if( it->is(VT::O::SC::C::A, VT::O::GENERATE) ) {
                movi(7,0);
            } else if( it->is(VN::FINAL) ) {
                redu(1, VN::FINAL);
            } else {
                return diagnostics("21", VT::O::GENERATE, *it), nullptr;
            } break;
        case 7:
            if( it->is(VT::O::SC::C::A) ) {
                if( call->ret_proto ) redu(1, VN::FINAL);
                else if( auto p = constructElementPrototype(scope, true); p )  {
                    call->ret_proto = p;
                } else {
                    return nullptr;
                }
            } else if( it->is(VT::O::GENERATE) ) {
                stay();
                if( call->ret_proto ) {
                    return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
                } else if( auto p = constructElementPrototype(scope, false); p ) {
                    call->ret_proto = p;
                } else {
                    return nullptr;
                }
            } else if( it->is(VN::ELEPROTO) ) {
                stay();
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), nullptr;
            }break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$constant SyntaxContext::constructConstantExpression( $scope scope ) {
    $constant ref = new constant;
    ref->setScope(scope);
    ref->etype = exprstmt::constant;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(
                VT::L::CHAR,VT::L::STRING,
                VT::L::FALSE,VT::L::TRUE,
                VT::L::FLOAT,VT::L::NULL,
                VT::L::I::B,VT::L::I::H,VT::L::I::N,VT::L::I::O) ) {
                    ref->value = *it;
                redu(1, VN::EXPRSTMT);
            } else if( it->is(VT::L::THIS) ) {
                ref->value = *it;
                movi(2);
            } else {
                return diagnostics("57", *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::CLASS) ) {
                ref->value = *it;
                redu(2,VN::EXPRSTMT);
            } else {
                redu(-2, VN::EXPRSTMT);
            } break;
        default:
            return internal_error, nullptr;
    }
    
    ref->phrase = *it;
    return ref;
}

$lctorexpr SyntaxContext::constructListConstructingExpression( $scope scope ) {
    $lctorexpr ref = new lctorexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::lctor;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::SC::O::L) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::O::SC::O::L, *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::CLASS) ) {
                if( ref->type_indicator or ref->size() ) return diagnostics("50", *it), nullptr;
                ref->type_indicator = constructNameExpression(scope, false);
                if( !ref->type_indicator ) return nullptr;
            } else if( it->is(VN::EXPRSTMT) ) {
                movi(3);
            } else if( it->is(VN::MORE) ) {
                stay();
                if( it->is(VT::O::SC::C::L) ) return diagnostics("50", *it), nullptr;
            } else if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                *ref << expr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::COMMA) ) {
                redu(1, VN::MORE);
            } else if( it->is(VT::O::SC::C::L) ) {
                redu(3, VN::EXPRSTMT);
            } else {
                return diagnostics("21", VT::O::SC::COMMA, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$sctorexpr SyntaxContext::constructStructuralConstructingExpression( $scope scope ) {
    $sctorexpr ref = new sctorexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::sctor;
    token name;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::SC::O::S) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::O::SC::O::S, *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::CLASS) ) {
                if( ref->type_indicator or ref->size() ) return diagnostics("49", *it), nullptr;
                ref->type_indicator = constructNameExpression(scope, false);
                if( !ref->type_indicator ) return nullptr;
            } else if( it->is(VT::L::LABEL) ) {
                name = *it;
                movi(3);
            } else if( it->is(VN::MORE) ) {
                stay();
                if( !it->is(VT::L::LABEL) ) return diagnostics("49", *it), nullptr;
            } else if( it->is(VN::FINAL) ) {
                stay();
                if( !it->is(VT::O::SC::C::S) ) return diagnostics("21", VT::O::SC::C::S, *it), nullptr;
            } else if( it->is(VN::NAMEEXPR) ) {
                stay();
            } else if( it->is(VT::O::SC::C::S) ) {
                redu(2, VN::EXPRSTMT);
            } else {
                return diagnostics("49", *it), nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::COLON) ) {
                movi(4);
            } else {
                return diagnostics("21", VT::O::SC::COLON, *it), nullptr;
            } break;
        case 4:
            if( it->is(VN::EXPRSTMT) ) {
                movi(5);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                expr->name = name;
                *ref << expr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::O::SC::C::S) ) {
                redu(-3, VN::FINAL);
            } else if( it->is(VT::O::SC::COMMA) ) {
                redu(3, VN::MORE);
            } else {
                return diagnostics("21", VT::O::SC::C::S, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$tctorexpr SyntaxContext::constructTupleConstructingExpression( $scope scope ) {
    $tctorexpr ref = new tctorexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::tctor;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::LT) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::O::LT, *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::EXPRSTMT) ) {
                movi(3);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else if( auto expr = constructExpressionStatement(scope, true); expr ) {
                *ref << expr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VT::O::GT) ) {
                redu(3, VN::EXPRSTMT);
            } else if( it->is(VT::O::SC::COMMA) ) {
                redu(1, VN::MORE);
            } else {
                return diagnostics("21", VT::O::GT, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$lambdaexpr SyntaxContext::constructLambdaExpression( $scope scope ) {
    $lambdaexpr ref = new lambdaexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::lambda;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::CONV) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::O::CONV, *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::LIST) ) {
                movi(3);
            } else {
                if( !constructParameterList(scope, *ref, false) ) return nullptr;
            } break;
        case 3:
            if( it->is(VN::ELEPROTO) ) {
                movi(4);
            } else if( auto proto = constructElementPrototype(scope, true); proto ) {
                ref->ret_proto = proto;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::BLOCKSTMT) ) {
                redu(4, VN::EXPRSTMT);
            } else if( auto stmt = constructBlockStatement(ref); stmt ) {
                ref->body = stmt;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

$newexpr SyntaxContext::constructNewExpression( $scope scope ) {
    $newexpr ref = new newexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::newexpr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::NEW) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::NEW, *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::TYPEEXPR) ) {
                movi(3);
            } else if( auto type = constructTypeExpression(scope,false); type ) {
                if( type->is_type(UnknownType) ) return diagnostics("31", *it), nullptr;
                ref->ntype = type;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(CT::TERMINATOR) ) {
                redu(-3, VN::EXPRSTMT);
            } else if( it->is(VT::O::SC::O::L) ) {
                if( ref->array ) return diagnostics("67", *it), nullptr;
                movi(4);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else if( it->is(VN::EXPRSTMT) ) {
                redu(-3, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->init = expr;
            } else {
                return nullptr;
            } break;
        case 4:
            if( it->is(VN::EXPRSTMT) ) {
                movi(5);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->array = expr;
            } else {
                return nullptr;
            } break;
        case 5:
            if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::MORE);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}
$delexpr SyntaxContext::constructDeleteExpression( $scope scope ) {
    $delexpr ref = new delexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::delexpr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::DELETE) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::DELETE, *it), nullptr;
            } break;
        case 2:
            if( it->is(VT::O::SC::O::L) ) {
                movi(3);
            } else if( it->is(VN::MORE) ) {
                if( ref->array ) return diagnostics("67", *it), nullptr;
                ref->array = *it;
                stay();
            } else if( it->is(VN::EXPRSTMT) ) {
                redu(2, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                ref->target = expr;
            } else {
                return nullptr;
            } break;
        case 3:
            if( it->is(VT::O::SC::C::L) ) {
                redu(1, VN::MORE);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}
$doexpr SyntaxContext::constructDoExpression( $scope scope ) {
    $doexpr ref = new doexpr;
    ref->setScope(scope);
    ref->etype = exprstmt::doexpr;

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::DO) ) {
                movi(2);
            } else {
                return diagnostics("21", VT::DO, *it), nullptr;
            } break;
        case 2:
            if( it->is(VN::EXPRSTMT) ) {
                redu(2, VN::EXPRSTMT);
            } else if( auto expr = constructExpressionStatement(scope); expr ) {
                if( expr->etype != exprstmt::call ) return diagnostics("68", *it), nullptr;
            } else {
                return nullptr;
            } break;
        default:
            return internal_error, nullptr;
    }

    ref->phrase = *it;
    return ref;
}

bool SyntaxContext::constructOperatorLabel( $scope scope, token& subtitle, $eprototype& proto ) {

    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::SC::O::S) ) {
                movi(2);
            } else if( it->is(VT::O::SC::O::L) ) {
                movi(5);
            } else if( it->is(PVT::EQUAL) ) {
                movi(7);
            } else if( it->is(PVT::POINT) ) {
                movi(8);
            } else if( it->is(PVT::SHARP) ) {
                movi(9);
            } else if( it->is(VT::O::AS) ) {
                movi(10);
            } else if( it->is(CT::OP_LABEL) ) {
                movo();
            } else {
                return diagnostics("28", *it), false;
            } break;
        case 2:
            if( it->is(VT::O::ETC) ) {
                movi(3);
            } else if( it->is(VT::O::B::REV) ) {
                movi(4);
            } else {
                return diagnostics("21", VT::O::ETC, *it), false;
            } break;
        case 3:
            if( it->is(VT::O::SC::C::S) ) {
                redu(3, VN::OP::SCTOR);
            } else {
                return diagnostics("21", VT::O::SC::C::S, *it), false;
            } break;
        case 4:
            if( it->is(VT::O::SC::C::S) ) {
                redu(3, VN::OP::DTOR);
            } else {
                return diagnostics("21", VT::O::SC::C::S, *it), false;
            } break;
        case 5:
            if( it->is(VT::O::SC::C::L) ) {
                redu(2, VN::OP::INDEX);
            } else if( it->is(VT::O::ETC) ) {
                movi(6);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), false;
            } break;
        case 6:
            if( it->is(VT::O::SC::C::L) ) {
                redu(3, VN::OP::LCTOR);
            } else {
                return diagnostics("21", VT::O::SC::C::L, *it), false;
            } break;
        case 7:
            if( it->is(PVT::ADD) ) {
                redu(2, VN::OP::ASS::ADD);
            } else if( it->is(PVT::SUB) ) {
                redu(2, VN::OP::ASS::SUB);
            } else if( it->is(PVT::MUL) ) {
                redu(2, VN::OP::ASS::MUL);
            } else if( it->is(PVT::DIV) ) {
                redu(2, VN::OP::ASS::DIV);
            } else if( it->is(PVT::MOL) ) {
                redu(2, VN::OP::ASS::MOL);
            } else if( it->is(PVT::SHL) ) {
                redu(2, VN::OP::ASS::SHL);
            } else if( it->is(PVT::SHR) ) {
                redu(2, VN::OP::ASS::SHR);
            } else if( it->is(PVT::BITAND) ) {
                redu(2, VN::OP::ASS::BITAND);
            } else if( it->is(PVT::BITOR) ) {
                redu(2, VN::OP::ASS::BITOR);
            } else if( it->is(PVT::BITXOR) ) {
                redu(2, VN::OP::ASS::BITXOR);
            } else {
                redu(-2, VN::OP::ASSIGN);
            } break;
        case 8:
            if( it->is(VT::L::LABEL) ) {
                subtitle = *it;
                redu(2, VN::OP::MEMBER);
            } else {
                return diagnostics("28", *it), false;
            } break;
        case 9:
            if( it->is(VT::L::LABEL, VT::L::I::N) ) {
                subtitle = *it;
                redu(2, VN::OP::ASPECT);
            } else {
                return diagnostics("28", *it), false;
            } break;
        case 10:
            if( it->is(VN::ELEPROTO) ) {
                redu(2, VN::OP::AS);
            } else if( proto = constructElementPrototype(scope, false) ) {
                if( proto->dtype->is_type(UnknownType) ) return diagnostics("31", *it), false;
            } else {
                return false;
            } break;
        default:
            return internal_error, false;
    }

    return true;
}

bool SyntaxContext::constructParameterList( $scope scope, callable& ref, bool defv ) {
    enter();
    movi(1,0);
    while( working() ) switch( states[-1] ) {
        case 1:
            if( it->is(VT::O::SC::O::A) ) {
                movi(100);
            } else {
                return diagnostics("21", VT::O::SC::O::A, *it), false;
            } break;
        case 100:
            if( it->is(VT::O::SC::C::A) ) {
                redu(2, VN::LIST);
            } else {
                movi(110,0);
            } break;
        case 110:
            if( it->is(VN::ELEMENTSTMT) ) {
                movi(113);
            } else if( it->is(VN::FINAL) ) {
                movi(113);
            } else if( it->is(VN::MORE) ) {
                stay();
            } else if( it->is(VT::O::ETC) ) {
                ref.va_arg = *it;
                movi(114);
            } else if( auto ele = constructElementStatement(scope, false); ele ) {
                ref.arguments << ele;
            } else {
                return false;
            } break;
        case 113:
            if( it->is(VT::O::SC::COMMA) ) {
                if( ref.va_arg ) return diagnostics("21", VT::O::SC::C::A, *it), false;
                redu(1, VN::MORE);
            } else if(it->is(VT::O::SC::C::A) ) {
                redu(4, VN::LIST);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), false;
            } break;
        case 114:
            if( it->is(VT::L::LABEL) ) {
                ref.va_arg = *it;
                redu(1, VN::FINAL);
            } else if( it->is(VT::O::SC::C::A) ) {
                redu(-1, VN::FINAL);
            } else {
                return diagnostics("21", VT::O::SC::C::A, *it), false;
            } break;
        default:
            return internal_error, false;
    }

    bool ok = true;
    for( auto& arg : ref.arguments ) {
        for( auto& another : ref.arguments ) {
            if( &another == &arg ) break;
            if( arg->name.tx == another->name.tx ) {
                diagnostics("56", arg->name)
                    [-1]
                        (diagnostics.prefix(), "45", another->name);
                ok = false;
            }
        }
    }

    if( !defv ) for( auto& arg : ref.arguments ) {
        if( arg->init ) ok = (diagnostics("52", arg->init->phrase), false);
    }

    return ok;
}

$statement SyntaxContext::constructStatement( $scope scope, bool block, bool ele ) {
    $statement ref;

    if( it->is(CT::ELETYPE,VT::VAR,VT::CONST) ) {
        if( !ele ) return diagnostics("59", *it), nullptr;
        auto e = constructElementStatement(scope,false);
        if( e and !e->init and e->proto->dtype->is_type(UnknownType) )
            diagnostics("31", e->name);
        ref = e;
    } else if( it->is(VT::IF) ) {
        ref = constructBranchStatement(scope);
    } else if( it->is(VT::SWITCH) ) {
        ref = constructSwitchStatement(scope);
    } else if( it->is(VT::LOOP) ) {
        ref = constructLoopStatement(scope);
    } else if( it->is(VT::BREAK,VT::RETURN,VT::CONTINUE) ) {
        ref = constructFlowControlStatement(scope);
    } else if( it->is(VT::ASSUME) ) {
        ref = constructAssumeStatement(scope);
    } else if( it->is(VT::DO) ) {
        ref = constructDoStatement(scope);
    } else if( block and it->is(VT::O::SC::O::S) ) {
        ref = constructBlockStatement(scope);
    } else {
        ref = constructExpressionStatement(scope);
    }

    return ref;
}


int SyntaxContext::prio(const token& it) {
    int p = 0;  
    /**
     * 所有运算符
     * p越小,优先级越高
     */
    do {
        if( !it.is(CT::OPERATOR) ) break;
        p += 1;

        if( it.is(VT::O::MEMBER,VT::O::POINTER,VT::O::SHARP) )break;
        p += 1;
        //if( it.is(VN::INDEX) ) break;p += 1;                   //单目运算符    后
        if( it.is(VT::O::INCREMENT,VT::O::DECREMENT) ) break;
        p += 1;       //单目运算符    前后
        //if( it.is(VT::prePLUS, VT::preMINUS) ) break; 
        p += 1;    //单目运算符    前
        //if( it.is(VT::ADDRESS,VT::REFER) ) break;
        p += 1;         //单目运算符    前
        if( it.is(VT::O::NOT) ) break; 
        p += 1;                      //单目运算符    前
        if( it.is(VT::O::B::REV) ) break;
        p += 1;                      //单目运算符    前
        if( it.is(VT::O::SHL,VT::O::SHR) ) break;
        p += 1;               //双目运算符    左结合
        if( it.is(VT::O::B::AND) ) break;
        p += 1;                      //双目运算符    左结合
        if( it.is(VT::O::B::XOR) ) break;
        p += 1;
        if( it.is(VT::O::B::OR) ) break;
        p += 1;
        if( it.is(VT::O::MOL,VT::O::MUL,VT::O::DIV) ) break;
        p += 1;
        if( it.is(VT::O::PLUS,VT::O::MINUS) ) break;
        p += 1;
        if( it.is(VT::O::AS,VT::O::TREAT) ) break;
        // p += 1;
        // if( it.is(VT::RANGE) ) break;
        p += 1;
        if( it.is(CT::RELATION) ) break;
        p += 1;
        if( it.is(VT::O::AND) ) break;
        p += 1;
        if( it.is(VT::O::OR) ) break;
        p += 1;
        if( it.is(CT::ASSIGN) ) break;
        p += 1;
    }while( false );
    return p;
}

}

#endif