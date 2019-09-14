#ifndef __compiler_cpp__
#define __compiler_cpp__

#include "compiler.hpp"
#include "lexical.hpp"
#include "syntax.hpp"
#include "alioth.hpp"
#include "space.hpp"
#include <iostream>
#include <regex>

namespace alioth {

static bool isalioth( const string& name ) {
    char temp[] = {'h','t','o','i','l','a','.'};
    if( name.size() <= 7 ) return false;
    for( int i = 0, j = name.size()-1; i < 7; i++ )
        if( temp[i] != name[j-i] ) return false;
    return true;
}

AbstractCompiler::~AbstractCompiler() {
    if( diagnostics.size() ) {
        uostream os = nullptr;
        if( diagnosticsDestination.fd >= 0 ) os = SpaceEngine::OpenStreamForWrite(diagnosticsDestination.fd);
        else os = SpaceEngine::OpenStreamForWrite(diagnosticsDestination.uri);
        switch( diagnosticsMethod ) {
            case STRING:
                for( auto diagnostic : diagnosticEngine->printToString(diagnostics) )
                    *os << diagnostic << endl;
                break;
            case JSON:
                *os << diagnosticEngine->printToJson(diagnostics).toJsonString();
                break;
        }
    }

    if( spaceEngine ) delete spaceEngine;
    if( diagnosticEngine ) delete diagnosticEngine;
}
AbstractCompiler::AbstractCompiler( AbstractCompiler&& compiler ):
spaceEngine(compiler.spaceEngine),
diagnosticEngine(compiler.diagnosticEngine),
diagnosticsMethod(compiler.diagnosticsMethod),
diagnosticsDestination(compiler.diagnosticsDestination),
diagnostics(move(diagnostics)) {
    compiler.spaceEngine = nullptr;
    compiler.diagnosticEngine = nullptr;
}

bool AbstractCompiler::configureDiagnosticsMethod( const string& m ) {
    if( m == "string" ) {
        diagnosticsMethod = STRING;
    } else if( m == "json" ) {
        diagnosticsMethod = JSON;
    } else {
        diagnostics["command-line"]("11",m);
        return false;
    }
    return true;
}

bool AbstractCompiler::configureDiagnosticsDestination( const string& d ) {
    regex reg(R"(\d+)");
    if( regex_match( d, reg ) ) {
        diagnosticsDestination.fd = stol(d);
    } else try {
        diagnosticsDestination.fd = -1;
        diagnosticsDestination.uri = Uri::FromString(d);
    } catch( exception& e ) {
        diagnostics["command-line"]("12",d);
        return false;
    }
    return true;
}


BasicCompiler::BasicCompiler( int argc, char **argv ) {

    /** 扫描命令行参数,整理成方便操作的序列 */
    for( int i = 1; i < argc; i++ ) args.construct(-1,argv[i]);
}

int BasicCompiler::execute() {
    bool success = true;
    bool root_remapping_failed = false;
    bool diagnostic_engine_failed = false;
    spaceEngine = new SpaceEngine;
    diagnosticEngine = new DiagnosticEngine;

    /** 扫描命令行参数，配置空间引擎 */
    for( int i = 0; i < args.size(); i++ ) {
        const auto arg = args[i];
        if( arg == "--work" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !spaceEngine->setMainSpaceMapping( SpaceEngine::WORK, args[i] ) ) {
                    diagnostics[args[i]]("3");
                    success = false;
                }
                args.remove(i--);
            }
        } else if( arg == "--root" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !spaceEngine->setMainSpaceMapping( SpaceEngine::ROOT, args[i] ) ) {
                    diagnostics[args[i]]("4");
                    success = false;
                    root_remapping_failed = true;
                }
                args.remove(i--);
            }
        } else if( arg == "--" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                auto reg = regex(R"((\d+)/(\d+))");
                if( !regex_match( args[i], reg ) ) {
                    diagnostics["command-line"]("6",args[i]);
                    success = false;
                } else {
                    size_t size;
                    auto in = stol(args[i],&size);
                    auto out = stol( string(args[i].begin()+size+1,args[i].end()) );
                    spaceEngine->enableInteractiveMode( in, out );
                }
                args.remove(i--);
            }
        }
    }

    /** 尝试配置诊断引擎 */
    auto diagnostic_desc = (SpaceEngine::Desc){
        flags: SpaceEngine::DOCUMENT | SpaceEngine::ROOT | SpaceEngine::DOC,
        name: "diagnostic.json" };
    diagnostics[spaceEngine->getUri(diagnostic_desc)];
    if( auto file = spaceEngine->openDocumentForRead(diagnostic_desc); file and !root_remapping_failed ) {
        try {
            auto config = json::FromJsonStream(*file);
            if( !diagnosticEngine->configure(config) ) {
                diagnostics("1");
                success = false;
                diagnostic_engine_failed = true;
            }
        } catch( exception& e ) {
            diagnostics("1");
            success = false;
            diagnostic_engine_failed = true;
        }
    } else if( !root_remapping_failed ) {
        diagnostics("0");
        success = false;
    }
    /** 避免失败的配置给诊断引擎带来缺陷，重置诊断引擎 */
    if( diagnostic_engine_failed )
        *spaceEngine = SpaceEngine();

    /** 读取命令行中与诊断引擎有关的配置信息 */
    for( int i = 0; i < args.size(); i++ ) {
        const auto arg = args[i];
        if( arg == "--diagnostics-format" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                diagnosticEngine->configureFormat(args[i]);
                args.remove(i--);
            }
        } else if( arg == "--diagnostics-lang" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !diagnosticEngine->selectLanguage(args[i]) ) {
                    diagnostics["command-line"]("5",arg,args[i]);
                    success = false;
                }
                args.remove(i--);
            }
        } else if( arg == "--diagnostics-method" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !configureDiagnosticsMethod(args[i]) ) success = false;
                args.remove(i--);
            }
        } else if( arg == "--diagnostics-to" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !configureDiagnosticsDestination(args[i]) ) {
                    diagnostics["command-line"]("12",args[i]);
                    success = false;
                }
                args.remove(i--);
            }
        }
    }

    /** 扫描先行目标,构造编译器 */
    AbstractCompiler* compiler = nullptr;
    if( success ) for( int i = 0; i < args.size(); i++ ) {
        const auto arg = args[i];
        if( arg == "--help" ) {
            args.remove(i);
            return help();
        } else if( arg == "--version" ) {
            args.remove(i);
            return version();
        } else if( arg == "--init" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("8",arg);
                success = false;
                break;
            } else {
                return init(args[i]);
            }
        } else if( arg == ":" or arg == "x:" or arg == "s:" or arg == "d:" or arg == "v:" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("13",arg);
                success = false;
            } else {
                CompilingTarget target;
                target.name = args[i];
                args.remove(i);
                target.modules = args;
                switch( arg[0] ) {
                    case ':': target.indicator = Target::AUTO; break;
                    case 'x': target.indicator = Target::EXECUTABLE; break;
                    case 's': target.indicator = Target::STATIC; break;
                    case 'd': target.indicator = Target::DYNAMIC; break;
                    case 'v': 
                        target.indicator = Target::VALIDATE;
                        if( !configureDiagnosticsDestination(target.name) ) success = false;
                        break;
                }
                if( success ) compiler = new AliothCompiler(*this,target);
            }
            break;
        } else if( arg == "package:" ) {

        } else if( arg == "install:" ) {

        } else if( arg == "update:" ) {

        } else if( arg == "remove:" ) {

        } else if( arg == "publish:" ) {

        }
    }

    if( !compiler ) {
        diagnostics["command-line"]("7");
        return success?0:1;
    } else {
        auto r = compiler->execute();
        delete compiler;
        return r;
    }
}

int BasicCompiler::help() {
    cout << R"(Please refer to the document named 'Alioth Compiler Manual' for help.)" << endl;
    return 0;
}

int BasicCompiler::version() {
    cout << "alioth (x86_64) " << __compiler_ver_str__ << endl
        << "Copyright (C) 2019 GodGnidoc" << endl
        << "corresponding language version: " << __language_ver_str__ << endl;
    return 0;
}

int BasicCompiler::init( const string& package ) {
    bool correct = true;

    diagnostics[spaceEngine->getUri({flags:SpaceEngine::WORK})];
    if( !spaceEngine->createSubSpace({flags: SpaceEngine::EXT|SpaceEngine::WORK, name: package }) ) {
        diagnostics("9");
        correct = false;
    }
    for( auto name : {"arc","bin","doc","inc","lib","obj","src"} )
        if( !spaceEngine->createSubSpace({flags: SpaceEngine::EXT|SpaceEngine::WORK, name: package + SpaceEngine::dirdvs + name }) ) {
            diagnostics("10",name);
            correct = false;
        }
    
    return correct?0:1;
}

BasicCompiler::~BasicCompiler() {
}

AliothCompiler::AliothCompiler( AbstractCompiler& basic, CompilingTarget compilingTarget ):
AbstractCompiler(move(basic)),target(compilingTarget) {

}

AliothCompiler::~AliothCompiler() {

}

int AliothCompiler::execute() {
    if( !detectInvolvedModules() ) return 1;

    return 0;
}

bool AliothCompiler::syncSignatures( SpaceEngine::Desc space ) {
    if( !space.isMainSpace() ) return internal_error, false;
    diagnostics[spaceEngine->getUri(space)];

    map<string,$signature>* cache_map = nullptr;
    /** 根据空间描述符清理缓冲空间 */
    switch( SpaceEngine::PeekMain(space.flags) ) {
        case SpaceEngine::WORK: cache_map = &work; break;
        case SpaceEngine::ROOT: cache_map = &root; break;
        case SpaceEngine::APKG: cache_map = &package[space.name]; break;
    } cache_map->clear();

    /** 读取缓冲文件 */
    if( !spaceEngine->reachDataSource(space) )
        return diagnostics("18", spaceEngine->getUri(space)), false;
    SpaceEngine::Desc cache_desc = {
        flags: space.flags|SpaceEngine::DOC|SpaceEngine::DOCUMENT,
        name: "space.json",
        package: space.package
    };
    do if( auto cache_stream = spaceEngine->openDocumentForRead(cache_desc); cache_stream ) {
        json cache_data;
        try{cache_data = json::FromJsonStream(*cache_stream);} 
        catch( exception& e ) {break;}
        if( !cache_data.is(json::object) ) break;
        if( auto& modules = cache_data["modules"]; modules.is(json::object) ) modules.for_each([&]( const string& name, json& module ) {
            auto& sig = (*cache_map)[name];
            if( !sig ) sig = new signature;
            sig->name = name;
            sig->space = space;
            if( auto deps = module["deps"]; deps.is(json::array) ) for( auto& dep : deps ) {
                auto desc = sig->deps.construct(-1,new depdesc);
                if( auto arg = dep["name"]; arg.is(json::string) ) desc->name = token(arg);
                if( auto arg = dep["alias"]; arg.is(json::string) ) {desc->alias = token(arg);if(desc->alias.tx == "this") desc->alias.id = VT::L::THIS;}
                if( auto arg = dep["from"]; arg.is(json::string) ) {desc->from = token(arg);if(desc->from.tx == ".") desc->alias.id = VT::O::MEMBER;}
                desc->setScope(sig);
            }
            if( auto code = module["code"]; code.is(json::array) ) for( auto& desc : code ) {
                auto& d = sig->docs.construct(-1);
                d.flags = space.flags | (long)desc["flags"],
                d.name = desc["name"];
                d.package = space.package;
                d.mtime = (time_t)(long)desc["mtime"];
                d.size = (size_t)(long)desc["size"];
            }
            return true;
        });
    } while( false );

    /** 接下来检查每个源代码登记是否过期，是否缺失，并根据此信息对信息不完整的模块重新扫描 */
    auto all_descs = spaceEngine->enumerateContents({
        flags: space.flags|SpaceEngine::SRC,
        package: space.package
    }) + spaceEngine->enumerateContents({
        flags: space.flags|SpaceEngine::INC,
        package: space.package
    });

    bool different = false;
    chainz<SpaceEngine::FullDesc> all_cached;
    for( auto [_,cache] : *cache_map ) all_cached += cache->docs;
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
            auto is = spaceEngine->openDocumentForRead( desc );
            if( !is and src ) {diagnostics("15",spaceEngine->getUri(desc)); continue;}
            auto lcontext = LexicalContext( *is, true );
            auto tokens = lcontext.perform();
            auto scontext = SyntaxContext(tokens, diagnostics);
            auto sig = scontext.extractSignature(src);
            if( !sig ) {if(src) success = false; continue;}
            if( cache_map->count(sig->name) ) {
                auto& mod = (*cache_map)[sig->name];
                mod->deps += sig->deps;
                mod->docs << desc;
            } else {
                sig->docs << desc;
                sig->space = space;
                (*cache_map)[sig->name] = sig;
            }
        }

        /** 将更新过的签名信息写入space缓冲文件 */
        json cache_data = json::object;
        auto& modules = cache_data["modules"] = json(json::object);
        for( auto& [name,sig] : *cache_map ) {
            auto& module = modules[name] = json(json::object);
            auto& deps = module["deps"] = json(json::array);
            auto& code = module["code"] = json(json::array);
            for( auto& desc : sig->docs ) code[code.count()] = desc.toJson();
            for( auto& dep : sig->deps ) deps[deps.count()] = dep->toJson();
        }
        auto os = spaceEngine->openDocumentForWrite(cache_desc);
        if( os ) *os << cache_data.toJsonString();
    }
    return success;
}

bool AliothCompiler::detectInvolvedModules() {
    bool success = true;
    if( !syncSignatures( {flags:SpaceEngine::WORK} ) ) return false;

    if( target.modules.size() ==  0 ) for( auto& [name,_] : work ) target.modules << name;

    for( auto& name : target.modules )
        success = confirmModuleCompleteness( name ) and success;
    
    return success;
}

bool AliothCompiler::confirmModuleCompleteness( const string& name ) {
    diagnostics[spaceEngine->getUri({flags:SpaceEngine::WORK})];
    if( work.count(name) == 0 ) return diagnostics("14",name), false;
    
    return confirmModuleCompleteness(work[name]);
}

bool AliothCompiler::confirmModuleCompleteness( $signature mod, chainz<$signature> padding ) {

    bool correct = true;
    auto space = spaceEngine->getUri(mod->space);
    diagnostics[space];
    /** 检查循环依赖 */
    for( auto& sig : padding ) {
        if( sig == mod ) {
            diagnostics("16", space, sig->name );
            for( auto& s : padding ) {
                space = spaceEngine->getUri(s->space);
                diagnostics[-1](space, "17", space, s->name );
                if( s == sig ) break;
            }
            return false;
        }
    } padding.insert( mod, 0 );

    for( auto& dep : mod->deps ) {
        bool repeat = false;
        /** 检查依赖可达性 */
        auto [sig,sp] = calculateDependencySignature(dep);  //解算依赖空间
        if( !sig ) {
            diagnostics("20", mod->name, space, dep->name, spaceEngine->getUri(sp) ); 
            correct = false; 
            continue;
        }
        /** 检查依赖重复 */
        for( auto i = mod->deps.begin(); &*i != &dep; i++ ) {
            auto [sigi,_] = calculateDependencySignature(*i);
            if( sig == sigi ) {
                diagnostics[(string)mod->name+" @ "+(string)space]( "19", dep->name, spaceEngine->getUri(sp) );
                correct = false;
                repeat = true;
            }
        }
        /** 检查依赖完备性 */
        if( !repeat ) correct = confirmModuleCompleteness(sig, padding) and correct;
    }

    /** 若模块完备，加入到目标模块队列中 */
    if( correct ) {
        bool found = false;
        for( auto& sig : target_modules ) if( sig == mod ) {found = true; break;}
        if( !found ) target_modules << mod;
    }

    return correct;
}

SpaceEngine::Desc AliothCompiler::calculateDependencySpace( $depdesc desc ) {
    auto [success,from,ds] = desc->from.extractContent();
    map<string,$signature> *cache = nullptr;
    $signature sig = desc->getScope();
    if( !sig ) return SpaceEngine::Desc::Error;
    auto local_desc = sig->space;
    switch( SpaceEngine::PeekMain(local_desc.flags) ) {
        case SpaceEngine::WORK: cache = &work; break;
        case SpaceEngine::ROOT: cache = &root; break;
        case SpaceEngine::APKG: cache = &package[local_desc.package];
        default: return SpaceEngine::Desc::Error;
    }
    if( !success ) return (diagnostics += ds),SpaceEngine::Desc::Error;
    if( from == "." ) {
        return local_desc;
    } else if( from == "alioth" ) {
        return {flags:SpaceEngine::ROOT};
    } else if( from.size() ) {
        return {flags:SpaceEngine::APKG,package:from};
    } else if( cache->count(desc->name) ) {
        return local_desc;
    } else {
        return {flags:SpaceEngine::ROOT};
    }
}

tuple<$signature,SpaceEngine::Desc> AliothCompiler::calculateDependencySignature( $depdesc desc ) {
    if( dep_cache.count(desc) ) return dep_cache[desc];
    auto space = calculateDependencySpace( desc );
    if( !space ) { internal_error; return {nullptr,space};}
    map<string,$signature> *cache_ptr = nullptr;
    switch( SpaceEngine::PeekMain(space.flags) ) {
        case SpaceEngine::WORK: cache_ptr = &work; break;
        case SpaceEngine::ROOT: cache_ptr = &root; break;
        case SpaceEngine::APKG: cache_ptr = &package[space.package]; break;
        default: return {nullptr,space};
    }

    if( cache_ptr->size() == 0 ) 
        if( !syncSignatures(space) ) return {nullptr,space};
    if( cache_ptr->count(desc->name) )
        return dep_cache[desc] = {(*cache_ptr)[desc->name],space};
    return {nullptr,space};
}

}

#endif