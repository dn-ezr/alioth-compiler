#ifndef __compiler__
#define __compiler__

#include "jsonz.hpp"
#include "target.hpp"
#include "space.hpp"
#include "diagnostic.hpp"

namespace alioth {

/**
 * @class AbstractCompiler : 抽象编译器
 * @desc :
 *  抽象编译器作为基类，统一所有编译器内核的共性。
 */
class AbstractCompiler {

    protected:
        enum DiagnosticsMethod {
            STRING,
            JSON
        };

        struct DiagnosticsDestination {
            int fd;
            Uri uri;
        };

    protected:
        SpaceEngine* spaceEngine = nullptr;
        DiagnosticEngine* diagnosticEngine = nullptr;
        DiagnosticsMethod diagnosticsMethod = STRING;
        DiagnosticsDestination diagnosticsDestination = {fd:1};
        Diagnostics diagnostics;

    public:
        AbstractCompiler() = default;
        AbstractCompiler( AbstractCompiler&& compiler );
        virtual ~AbstractCompiler() = 0;
        virtual int execute() = 0;

        bool configureDiagnosticsMethod( const string& m );
        bool configureDiagnosticsDestination( const string& d );
};

/**
 * @class BasicCompiler : 基础编译器
 * @desc :
 *  基础编译器用于在确定目标之前启动基础资源配置
 *  基础编译器也用于基于磁盘路径工作
 */
class BasicCompiler : public AbstractCompiler {
    
    private:
        chainz<string> args;
        Diagnostics diagnostics;

    public:
        BasicCompiler( int argc, char** argv );
        ~BasicCompiler();
        int execute() override;

        int version();
        int help();
        int init( const string& package );

};

/**
 * @class AliothCompiler : 语言编译器
 * @desc :
 *  用于执行需要编译流程的目标的编译器
 *  如Auto,Executable,Static,Dynamic,Validate
 */
class AliothCompiler : public AbstractCompiler {

    private:
        CompilingTarget target;

    public:
        AliothCompiler( AbstractCompiler& basic, CompilingTarget compilingTarget );
        ~AliothCompiler();
        int execute() override;
    
};

class PackageManager : public AbstractCompiler {

};

class RepositoryManager : public AbstractCompiler {

};

}

#endif