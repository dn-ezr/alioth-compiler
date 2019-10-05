#ifndef __context_cpp__
#define __context_cpp__

#include "context.hpp"
#include "lexical.hpp"

namespace alioth {

static bool isalioth( const string& name ) {
    char temp[] = {'h','t','o','i','l','a','.'};
    if( name.size() <= 7 ) return false;
    for( int i = 0, j = name.size()-1; i < 7; i++ )
        if( temp[i] != name[j-i] ) return false;
    return true;
}

CompilerContext::CompilerContext( SpaceEngine& _spaceEngine, Diagnostics& _diagnostics ):
    spaceEngine(_spaceEngine),diagnostics(_diagnostics) {

}

$signature CompilerContext::getModule( const string& name, srcdesc space, bool autogen ) {
    if( !token::islabel(name) ) return nullptr;
    map<string, $signature> *space_map = nullptr;
    switch( SpaceEngine::PeekMain(space.flags) ) {
        case WORK: space_map = &work; break;
        case ROOT: space_map = &root; break;
        case APKG: if( package.count(space.package) or autogen )
                {space_map = &package[space.package];break;}
            [[fallthrough]]
        default: return nullptr;
    }
    if( space_map->count(name) ) {
        return space_map->at(name);
    } else if( autogen ) {
        auto& sig = (*space_map)[name] = new signature;
        sig->name = name;
        sig->space = {
            flags:SpaceEngine::PeekMain(space.flags),
            package: space.package
        };
        return sig;
    } else {
        return nullptr;
    }
}

$signature CompilerContext::getModule( srcdesc doc ) {
    /** 
     * 2019/09/22在原本的设计中，在Context存在专门的map<fulldesc,$signature>用于存储此映射关系
     * 但是考虑到srcdesc出现次数太多，不方便统一优先权，故放弃此设计 */
    map<string,$signature> *map = nullptr;
    if( !doc ) return nullptr;
    switch( SpaceEngine::PeekMain(doc.flags) ) {
        case WORK: map = &work; break;
        case ROOT: map = &root; break;
        case APKG: if( package.count(doc.package) ) 
                {map = &package[doc.package]; break;}
            [[fallthrough]]
        default: return nullptr;
    }
    for( auto& [_,mod] : *map ) 
        if( mod->docs.count(doc) ) return mod;
    return nullptr;
}

$signature CompilerContext::getModule( $fragment fg ) {
    if( !fg or !fg->doc ) return nullptr;
    return getModule( fg->doc );
}

signatures CompilerContext::getModules( srcdesc space ) {
    space.flags = SpaceEngine::PeekMain(space.flags);
    map<string,$signature> *map = nullptr;
    switch( SpaceEngine::PeekMain(space.flags) ) {
        case WORK: map = &work; break;
        case ROOT: map = &root; break;
        case APKG: if( package.count(space.package) ) 
                {map = &package[space.package]; break;}
            [[fallthrough]]
        default: return {};
    }
    signatures sigs;
    for( auto [_,sig] : *map ) sigs << sig;
    return sigs;
}

size_t CompilerContext::countModles( srcdesc space ) {
    space.flags = SpaceEngine::PeekMain(space.flags);
    map<string,$signature> *map = nullptr;
    switch( SpaceEngine::PeekMain(space.flags) ) {
        case WORK: map = &work; break;
        case ROOT: map = &root; break;
        case APKG: if( package.count(space.package) ) 
                {map = &package[space.package]; break;}
            [[fallthrough]]
        default: return 0;
    }
    return map->size();
}

bool CompilerContext::deleteModule( $signature sig ) {
    if( !sig ) return false;
    map<string,$signature> *map = nullptr;
    switch( SpaceEngine::PeekMain(sig->space.flags) ) {
        case WORK: map = &work; break;
        case ROOT: map = &root; break;
        case APKG: if( package.count(sig->space.package) ) 
                {map = &package[sig->space.package]; break;}
            [[fallthrough]]
        default: return false;
    }
    if( map->count(sig->name) ) {
        map->erase(sig->name);
        return true;
    }
    return false;
}

bool CompilerContext::loadModules( srcdesc space ) {
    if( !space or !space.isMainSpace() ) return internal_error, false;
    diagnostics[spaceEngine.getUri(space)];
    json cache_data;

    map<string,$signature> *cache_map = nullptr;
    /** 根据空间描述符清理缓冲空间 */
    clearSpace(space);
    switch( space.flags = SpaceEngine::PeekMain(space.flags) ) {
        case WORK: cache_map = &work; break;
        case ROOT: cache_map = &root; break;
        case APKG: cache_map = &package[space.name]; break;
    }

    /** 读取缓冲文件 */
    if( !spaceEngine.reachDataSource(space) )
        return diagnostics("18", spaceEngine.getUri(space)), false;
    srcdesc cache_desc = {
        flags: space.flags|DOC|DOCUMENT,
        name: "space.json",
        package: space.package
    };
    do if( auto cache_stream = spaceEngine.openDocumentForRead(cache_desc); cache_stream ) {
        try{cache_data = json::FromJsonStream(*cache_stream);} 
        catch( exception& e ) {break;}

        /** 读取缓冲数据 */
        if( !cache_data.is(json::object) ) break;
        if( auto& modules = cache_data["modules"]; modules.is(json::object) ) modules.for_each([&]( const string& name, json& module ) {
            auto sig = signature::fromJson(module,space);
            if( !sig ) return true;
            auto& origin = (*cache_map)[name];
            if( !origin ) origin = sig;
            else origin->combine(sig);
            return true;
        });
    } while( false );

    /** 接下来检查每个源代码登记是否过期，是否缺失，并根据此信息对信息不完整的模块重新扫描 */
    chainz<fulldesc> all_descs;
    if( srcdesc desc = {flags: space.flags|SRC,package: space.package}; 
        spaceEngine.reachDataSource(desc) ) 
            all_descs = spaceEngine.enumerateContents(desc); 
    if( srcdesc desc = {flags: space.flags|INC,package: space.package}; 
        spaceEngine.reachDataSource(desc) )
            all_descs += spaceEngine.enumerateContents(desc);

    bool different = false;
    chainz<fulldesc> all_cached;
    for( auto [_,cache] : *cache_map )
        for( auto [doc,_] : cache->docs )
            all_cached << doc;
    auto all_existing = all_descs;
    for( auto& cached : all_cached ) {
        bool found = false;
        for( auto ie = all_existing.begin(); ie != all_existing.end(); ie++ ) {
            if( cached.flags != ie->flags or cached.name != ie->name ) continue;
            found = true;
            if( cached.size != ie->size or cached.mtime != ie->mtime ) different = true;
            all_existing.remover(*ie--);
            break;
        }
        if( !found ) different = true;
    } if( all_existing.size() != 0 ) different = true;

    /** 若源码发生任何变化，重新读取所有文档的模块签名，填写签名信息之后，写入缓冲配置文件。 */
    bool success = true;
    if( different ) {
        cache_map->clear();

        for( const auto& desc : all_descs ) {
            auto src = isalioth(desc.name);
            auto x = loadDocument(desc);
            if( src and !x ) success = false;
        }

        /** 将更新过的签名信息写入space缓冲文件 */
        if( !cache_data.is(json::object) ) cache_data = json(json::object);
        auto& modules = cache_data["modules"] = json(json::object);
        for( auto& [name,sig] : *cache_map ) {
            modules[name] = sig->toJson();
        }
        auto os = spaceEngine.openDocumentForWrite(cache_desc);
        if( os ) *os << cache_data.toJsonString();
    }
    return success;
}

bool CompilerContext::clearSpace( srcdesc space ) {
    space.flags = SpaceEngine::PeekMain(space.flags);
    switch( space.flags ) {
        case WORK: work.clear(); break;
        case ROOT: root.clear(); break;
        case APKG:
            if( package.count(space.package) ) { package.erase(space.package); break;}
        [[fallthrough]]
        default: return false;
    }
    return true;
}

fulldesc CompilerContext::loadDocument( srcdesc doc, bool forceload ) {
    return loadDocument(spaceEngine.statDataSource(doc),forceload);
}

fulldesc CompilerContext::loadDocument( fulldesc doc, bool forceload ) {
    auto module = getModule(doc);
    if( module and !forceload ) {
        if( auto it = module->docs.find(doc); 
            it != module->docs.end() and it->first.mtime >= doc.mtime ) 
                return it->first;
    }
    auto src = isalioth(doc.name);
    auto is = spaceEngine.openDocumentForRead( doc );
    if( !is ) {
        if( module ) module->docs.erase(doc);
        if( src ) {
            diagnostics("15",spaceEngine.getUri(doc)); 
            return srcdesc::error;
        }
    }
    auto lcontext = LexicalContext( *is, true );
    auto tokens = lcontext.perform();
    auto scontext = SyntaxContext(tokens, diagnostics);
    auto sig = scontext.extractSignature(src);
    if( !sig ) {
        if( module ) module->docs.erase(doc);
        return srcdesc::error;
    }
    sig->space.flags = SpaceEngine::PeekMain(doc.flags);
    sig->space.package = doc.package;
    if( !module ) module = getModule( sig->name, doc, true );
    module->combine(sig);
    module->docs[doc] = nullptr;
    return doc;
}

bool CompilerContext::unloadDocument( srcdesc doc, bool keepmodule ) {
    auto mod = getModule(doc);
    if( !mod ) return false;
    if( !mod->docs.erase(doc) ) return false;
    if( !keepmodule and mod->docs.empty() == 0 )
        deleteModule(mod);
    return true;
}

bool CompilerContext::registerFragment( srcdesc doc, $fragment fg ) {
    auto mod = getModule(doc);
    if( !mod or !mod->docs.count(doc) ) return false;
    mod->docs[doc] = fg;
    return true;
}

$fragment CompilerContext::getFragment( srcdesc doc ) {
    auto mod = getModule(doc);
    if( !mod ) return nullptr;
    if( auto it = mod->docs.find(doc); it != mod->docs.end() ) 
        return it->second;
    return nullptr;
}

}

#endif