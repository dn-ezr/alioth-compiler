#ifndef __semantic__
#define __semantic__

#include "syntax.hpp"
#include "context.hpp"

namespace alioth {

/**
 * @struct module : 模块
 * @desc : 模块对应于Alioth模块的概念，收集了属于一个模块的所有定义和实现 */
struct module : node {
    public:

        /**
         * @member sig : 签名 */
        $signature sig;

        /**
         * @member defs : 定义 */
        definitions defs;

        /**
         * @member impls : 实现 */
        implementations impls;

    public:
        virtual ~module() = default;
        bool is( type )const override;

        $module getModule()override;
        CompilerContext& getCompilerContext()override;
};

/**
 * @struct SemanticContext : 语义上下文
 * @desc :
 *  语义分析上下文用于进行语义分析，将抽象语法树转化为API模块
 */
class SemanticContext {

    private:

        /**
         * @member cctx : 编译器上下文 */
        CompilerContext& cctx;

        /**
         * @member diagnostics : 诊断信息容器 */
        Diagnostics& diagnostics;

        /**
         * @member forest : 语法树森林
         * @desc : 由抽象模块构成的森林 */
        map<$signature,$module> forest;

    public:

        SemanticContext( CompilerContext& context, Diagnostics& diagnostics_repo );

        /**
         * @method associateModule : 关联模块
         * @desc :
         *  将模块关联到语义上下文，在语义上下文环境中构造抽象模块对象
         */
        bool associateModule( $signature );

        /**
         * @method associateModules : 关联模块
         * @desc :
         *  批量关联模块
         */
        bool associateModules( signatures );

        /**
         * @method validateDefinitionSemantics : 检验定义语义
         * @desc :
         *  检查定义的语义正确性，过程中可能会修正一些语法结构
         */
        bool validateDefinitionSemantics();

        /**
         * @method validateImpelementationSemantics : 检验实现语义
         * @desc :
         *  检验实现语义的正确性，过程中可能会产生新的语法结构
         */
        bool validateImplementationSemantics();
};

}

#endif