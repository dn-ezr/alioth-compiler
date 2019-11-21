#ifndef __semantic__
#define __semantic__

#include "syntax.hpp"
#include "context.hpp"

namespace alioth {

class SemanticContext;

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

        /**
         * @member sctx : 语义上下文 */
        SemanticContext& sctx;

    public:
        module( SemanticContext& context );
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


        /**
         * @member symbol_cache : 二进制符号缓冲 */
        map<$node, string> symbol_cache;

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
         * @method releaseModule : 释放模块
         * @desc :
         *  将模块信息释放, 若模块未找到则失败
         */
        bool releaseModule( $signature );

        /**
         * @method releaseModules : 批量释放模块
         * @desc :
         *  批量释放模块信息
         */
        void releaseModules( signatures );

        /**
         * @method getModule : 获取模块
         * @desc :
         *  根据模块签名获取模块
         */
        $module getModule( $signature );

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

    private:
        bool validateModuleDefinition( $module );
        bool validateClassDefinition( $classdef );
        bool validateEnumDefinition( $enumdef );
        bool validateAliasDefinition( $aliasdef );
        bool validateAttributeDefinition( $attrdef );
        bool validateMethodDefinition( $metdef );
        bool validateOperatorDefinition( $opdef );

        bool validateMethodImplementation( $metimpl );
        bool validateOperatorImplementation( $opimpl );

        bool validateBlockStatement( $blockstmt );
        bool validateElementStatement( $element );
        bool validateFlowControlStatement( $fctrlstmt );
        bool validateExpressionStatement( $exprstmt );
        bool validateBranchStatement( $branchstmt );
        bool validateLoopStatement( $loopstmt );
        bool validateAssumeStatement( $assumestmt );
        bool validateDoStatement( $dostmt );

    public:

        /**
         * @static-method GetBinarySymbol : 获取二进制符号
         * @desc : 无论成功与否，被传入的语法结构应当已经被语义检查过程处理过 */
        static string GetBinarySymbol( $node );

        /**
         * @static-method Reach : 尝试抵达名称表达式
         * @return tuple<everything,bool> : 查询结果，是否遭遇错误 */
        static tuple<everything,bool> Reach( $nameexpr );
        
        /**
         * @static-method GetDefinition : 获取实现对应的定义 */
        static $definition GetDefinition( $implementation );

        /**
         * @method CanBeInstanced : 是否可实例
         * @desc :
         *  检查类定义是否可被实例化
         *  抽象类的所有方法都被实现才能实例化
         *  模板类的用例可以被实例化
         *  平坦类可以被实例化
         * 
         *  此过程辅助语义检查，却不依赖于语义检查结果
         *  也即，此方法仅以简单的规则运行，不过多考虑实际因素
         */
        static bool CanBeInstanced( $classdef );
};

}

#endif