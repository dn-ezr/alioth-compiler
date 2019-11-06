#ifndef __compiler_cpp__
#define __compiler_cpp__

#include "compiler.hpp"
#include "semantic.hpp"
#include "lexical.hpp"
#include "syntax.hpp"
#include "alioth.hpp"
#include "space.hpp"
#include <iostream>
#include <regex>

namespace alioth {
using namespace std;

AbstractCompiler::~AbstractCompiler() {
    if( diagnostics.size() ) {
        uostream os = nullptr;
        if( diagnosticDestination.fd >= 0 ) os = SpaceEngine::OpenStreamForWrite(diagnosticDestination.fd);
        else os = SpaceEngine::OpenStreamForWrite(diagnosticDestination.uri);
        switch( diagnosticMethod ) {
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
diagnosticMethod(compiler.diagnosticMethod),
diagnosticDestination(compiler.diagnosticDestination),
diagnostics(move(diagnostics)) {
    compiler.spaceEngine = nullptr;
    compiler.diagnosticEngine = nullptr;
}

bool AbstractCompiler::configureDiagnosticMethod( const string& m ) {
    if( m == "string" ) {
        diagnosticMethod = STRING;
    } else if( m == "json" ) {
        diagnosticMethod = JSON;
    } else {
        diagnostics["command-line"]("11",m);
        return false;
    }
    return true;
}

bool AbstractCompiler::configureDiagnosticDestination( const string& d ) {
    regex reg(R"(\d+)");
    if( regex_match( d, reg ) ) {
        diagnosticDestination.fd = stol(d);
    } else try {
        auto uri = Uri::FromString(d);
        if( uri ) {
            diagnosticDestination.uri = uri;
            diagnosticDestination.fd = -1;
        }
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
                if( !spaceEngine->setMainSpaceMapping( WORK, args[i] ) ) {
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
                if( !spaceEngine->setMainSpaceMapping( ROOT, args[i] ) ) {
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
    auto diagnostic_desc = (srcdesc){
        flags: DOCUMENT | ROOT | DOC,
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
        if( arg == "--diagnostic-format" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                diagnosticEngine->configureFormat(args[i]);
                args.remove(i--);
            }
        } else if( arg == "--diagnostic-lang" ) {
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
        } else if( arg == "--diagnostic-method" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !configureDiagnosticMethod(args[i]) ) success = false;
                args.remove(i--);
            }
        } else if( arg == "--diagnostic-to" ) {
            if( args.remove(i); i >= args.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                if( !configureDiagnosticDestination(args[i]) ) {
                    success = false;
                }
                args.remove(i--);
            }
        }
    }

    /** 扫描先行目标,构造编译器 */
    AbstractCompiler* compiler = nullptr;
    bool target_found = false;
    string target_option;
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
            target_found = true;
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
                    case 'v': {
                        target.indicator = Target::VALIDATE; 
                        success = configureDiagnosticDestination(target.name) and success;
                    } break;
                }
                if( success ) compiler = new AliothCompiler(*this,target);
            } break;
        } else if( arg == "package:" ) {
            target_found = true;
            target_option = arg;
        } else if( arg == "install:" ) {
            target_found = true;
            target_option = arg;
        } else if( arg == "update:" ) {
            target_found = true;
            target_option = arg;
        } else if( arg == "remove:" ) {
            target_found = true;
            target_option = arg;
        } else if( arg == "publish:" ) {
            target_found = true;
            target_option = arg;
        } else if( arg == "--gui" ) {
            target_found = true;
            target_option = arg;
        }
    }

    if( !target_found ) {
        diagnostics["command-line"]("7");
        return success?0:1;
    } else if( compiler ) {
        auto r = compiler->execute();
        delete compiler;
        return r;
    } else {
        diagnostics["command-line"]("tyet", target_option );
        return 1;
    }

    return 0;
}

int BasicCompiler::help() {
    cout << R"(Please refer to the document named 'Alioth Compiler Manual' for help.)" << endl;
    return 0;
}

int BasicCompiler::version() {
    cout << "alioth (linux-x86_64) " << __compiler_ver_str__ << endl
        << "  Copyright (C) 2019 GodGnidoc <stinger121@live.com>" << endl
        << "    The corresponding language version: " << __language_ver_str__ << endl;
    return 0;
}

int BasicCompiler::init( const string& package ) {
    bool correct = true;

    diagnostics[spaceEngine->getUri({flags:WORK})];
    if( !spaceEngine->createSubSpace({flags: EXT|WORK, name: package }) ) {
        diagnostics("9");
        correct = false;
    }
    for( auto name : {"arc","bin","doc","inc","lib","obj","src"} )
        if( !spaceEngine->createSubSpace({flags: EXT|WORK, name: package + SpaceEngine::dirdvs + name }) ) {
            diagnostics("10",name);
            correct = false;
        }
    
    return correct?0:1;
}

BasicCompiler::~BasicCompiler() {
}

AliothCompiler::AliothCompiler( AbstractCompiler& basic, CompilingTarget compilingTarget ):
AbstractCompiler(move(basic)),target(compilingTarget),context(*spaceEngine,diagnostics) {

}

AliothCompiler::~AliothCompiler() {

}

int AliothCompiler::execute() {

    bool success = true;

    for( int i = 0; i < target.modules.size(); i++ ) {
        const auto arg = target.modules[i];
        if( arg == "---" ) {
            if( target.modules.remove(i); i >= target.modules.size() ) {
                diagnostics["command-line"]("2",arg);
                success = false;
            } else {
                auto reg = regex(R"((\d+)/(\d+))");
                if( !regex_match( target.modules[i], reg ) ) {
                    diagnostics["command-line"]("47",target.modules[i]);
                    success = false;
                } else {
                    size_t size;
                    auto in = stol(target.modules[i],&size);
                    auto out = stol( string(target.modules[i].begin()+size+1,target.modules[i].end()) );

                    success = enableFullInteractiveMode( in, out );
                }
                target.modules.remove(i--);
            }
        }
    }if( !success ) return diagnostics("48"), 1;

    if( full_interactive ) return execute_full_interactive();
    
    context.loadModules({flags:WORK});
    if( !detectInvolvedModules() ) return 2;
    if( !performSyntaticAnalysis() ) return 3;
    if( !performSemanticAnalysis() ) return 4;
    if( target.indicator == Target::VALIDATE ) return 0;

    return 0;
}

int AliothCompiler::execute_full_interactive() {
    using namespace protocol;
    context.loadModules({flags:WORK});
    while( true ) try {
        diagnostics.clear();
        auto package = msock.receiveRequest();
        auto request = package->request();
        switch( package->title ) {
            case DIAGNOSTICS: {
                auto params = request->diagnostics();
                target.modules = params->targets;
                bool success = true;
                
                success = success and detectInvolvedModules();
                success = success and performSyntaticAnalysis();
                //[TODO] success = success and performSemanticAnalysis();
                
                auto info = diagnosticEngine->printToJson(diagnostics);
                msock.respondDiagnostics(package->seq, info);
            } break;
            case WORKSPACE: {
                auto params = request->workspace();

                spaceEngine->setMainSpaceMapping(WORK,params->uri);
                context.clearSpace({flags:MAIN});
                context.loadModules({flags:WORK});

                msock.respondSuccess(package->seq, WORKSPACE);
            } break;
            case EXIT: case NOMORE: {
                return 0;
            }
            default: break;
        }
    } catch( exception& e ) {
        msock.reportException(e.what());
    }

    return 0;
}

bool AliothCompiler::enableFullInteractiveMode( int input, int output ) {
    if( full_interactive ) return true;
    auto i = msock.ActivateInputStream({scheme:"fd",host:to_string(input),port:0});
    auto o = msock.ActivateOutputStream({scheme:"fd",host:to_string(output),port:0});
    auto s = spaceEngine->enableInteractiveMode(input,output);

    if( i and o and s ) full_interactive = true;
    return i and o and s;
}

bool AliothCompiler::detectInvolvedModules() {
    bool success = true;
    
    target_modules.clear();
    success = success and context.syncModules( {flags:WORK} );

    if( target.modules.size() ==  0 )
        for( auto& sig : context.getModules({flags:WORK}) ) 
            target.modules << sig->name;

    for( auto& name : target.modules )
        success = confirmModuleCompleteness( name ) and success;
    
    return success;
}

bool AliothCompiler::performSyntaticAnalysis() {
    bool success = true;
    for( auto s : target_modules )
        success = performSyntaticAnalysis(s) and success;
    return success;
}

bool AliothCompiler::performSemanticAnalysis() {
    auto semantic = SemanticContext( context, diagnostics );
    return false; // [TODO]
}

bool AliothCompiler::performSyntaticAnalysis( $signature sig ) {
    bool success = true;
    for( auto& [doc,_] : sig->docs ) {
        if( get<0>(_) != 0 ) {
            diagnostics += get<2>(_);
            continue;
        }
        Diagnostics tempd;
        tempd[spaceEngine->getUri(doc)];

        auto is = spaceEngine->openDocumentForRead( doc );
        if( !is ) {
            tempd("15", spaceEngine->getUri(doc));
            success = false;
        } else {
            auto lc = LexicalContext( *is, false );
            auto tokens = lc.perform();
            auto sc = SyntaxContext(tokens, tempd);
            auto fg = sc.constructFragment();
            if( fg ) context.registerFragment(doc,fg);
            else context.registerFragmentFailure(doc,tempd);
            if( !fg ) success = false;
        }
        diagnostics += tempd;
    }
    return success;
}

bool AliothCompiler::confirmModuleCompleteness( const string& name ) {
    diagnostics[spaceEngine->getUri({flags:WORK})];
    if( auto mod = context.getModule( name, {flags:WORK} ); !mod )
        return diagnostics("14",name), false;
    else
        return confirmModuleCompleteness(mod);
}

bool AliothCompiler::confirmModuleCompleteness( $signature mod, chainz<tuple<$signature,$depdesc>> padding, $depdesc _from ) {

    bool correct = true;
    auto space = spaceEngine->getUri(mod->space);
    diagnostics[space];
    /** 检查循环依赖 */
    for( auto& [sig,_] : padding ) {
        if( sig == mod ) {
            diagnostics[get<1>(padding[-2])->getDocUri()]("16", space, sig->name );
            for( auto& [s,f] : padding ) {
                space = spaceEngine->getUri(s->space);
                diagnostics[f->getDocUri()][-1](space, "17", space, s->name, f->phrase );
                if( s == sig ) break;
            }
            return false;
        }
    } padding.insert( {mod,_from}, 0 );

    for( auto& dep : mod->deps ) {
        bool repeat = false;
        /** 检查依赖可达性 */
        srcdesc sp;
        auto sig = calculateDependencySignature(dep,&sp);  //解算依赖空间
        if( !sig ) {
            if( dep->from.tx.size() ) {
                auto [suc,str,dia] = dep->from.extractContent();
                diagnostics[dep->getDocUri()]("20",mod->name, space, dep->name, str );
            } else {
                diagnostics[dep->getDocUri()]("20", mod->name, space, dep->name, "*" );
            }
            correct = false; 
            continue;
        }
        /** 检查依赖重复 */
        for( auto i = mod->deps.begin(); &*i != &dep; i++ ) {
            auto sigi = calculateDependencySignature(*i);
            if( sig == sigi ) {
                diagnostics[dep->getDocUri()]( "19", dep->name, spaceEngine->getUri(sp) );
                diagnostics[-1]((*i)->getDocUri(), "45", (*i)->phrase );
                correct = false;
                repeat = true;
            }
        }
        /** 检查依赖完备性 */
        if( !repeat ) correct = confirmModuleCompleteness(sig, padding,dep) and correct;
    }

    /** 若模块完备，加入到目标模块队列中 */
    if( correct ) {
        bool found = false;
        for( auto& sig : target_modules ) if( sig == mod ) {found = true; break;}
        if( !found ) target_modules << mod;
    }

    return correct;
}

srcdesc AliothCompiler::calculateDependencySpace( $depdesc desc ) {
    auto [success,from,ds] = desc->from.extractContent();
    $signature sig = desc->getScope();
    if( !sig ) return srcdesc::error;
    auto local_desc = sig->space;
    if( !success ) return (diagnostics += ds),srcdesc::error;
    desc->from.tx = from;
    desc->from.id = VT::L::LABEL;
    if( from == "." ) {
        return local_desc;
    } else if( from == "alioth" ) {
        return {flags:ROOT};
    } else if( from.size() ) {
        return {flags:APKG,package:from};
    } else if( context.getModule(desc->name, local_desc) ) {
        return local_desc;
    } else {
        return {flags:ROOT};
    }
}

$signature AliothCompiler::calculateDependencySignature( $depdesc desc, srcdesc* sp ) {
    auto space = calculateDependencySpace( desc );
    if( sp ) *sp = space;
    if( !space ) return internal_error,nullptr;

    if( context.countModles(space) == 0 ) 
        if( !context.loadModules(space) ) 
            return nullptr;

    return context.getModule(desc->name, space);
}

}

#endif