#ifndef __air_context__
#define __air_context__

#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>
#include "diagnostic.hpp"
#include "agent.hpp"

namespace alioth {

class SemanticContext;
class module;
using $module = agent<module>;

/**
 * @class AirContext : AIR上下文
 * @desc :
 *  AIR上下文用于在全局管理可能被重用的类型信息。
 *  AIR上下文用于在所有相关联的模块之间管理符号表。
 */
class AirContext : public llvm::LLVMContext {

    private:

        /**
         * @member diagnostics : 诊断容器 */
        Diagnostics& diagnostics;

        /**
         * @member arch : 架构 */
        string arch;

        /**
         * @member platform : 平台 */
        string platform;

        /**
         * @member targetTriple : 平台描述 */
        string targetTriple;

        /**
         * @member targetMachine : 描述目标机器 */
        llvm::TargetMachine* targetMachine;


    public:

        AirContext( string arch, string platform, Diagnostics& diag );

        ~AirContext();

        bool operator()( $module mod, ostream& os );

    private:
        llvm::Module* translateModule( $module );
};

}

#endif