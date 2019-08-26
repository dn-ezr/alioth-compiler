#ifndef __compiler_cpp__
#define __compiler_cpp__

#include "compiler.hpp"
#include "alioth.hpp"
#include "space.hpp"
#include <iostream>
#include <ext/stdio_filebuf.h>
#include <regex>

namespace alioth {

AbstractCompiler::~AbstractCompiler() {
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

AliothCompiler::AliothCompiler( AbstractCompiler& basic, CompilingTarget compilingTarget ):
AbstractCompiler(move(basic)),target(compilingTarget) {

}

AliothCompiler::~AliothCompiler() {

}

int AliothCompiler::execute() {
    return 0;
}

}

#endif