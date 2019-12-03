#ifndef __semantic__
#define __semantic__

#include "syntax.hpp"
#include "context.hpp"

namespace alioth {

class SemanticContext;

/**
 * @struct module : 模块
 * @desc : 模块对应于Alioth模块的概念，收集了属于一个模块的所有定义和实现 */
struct module : classdef {
    public:

        /**
         * @member sig : 签名 */
        $signature sig;

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
        $node clone( $scope scope )const override;
        this_is_scope

        $module getModule()override;
        CompilerContext& getCompilerContext()override;
};

/** 描述符号检索时的选项 */
using SearchOptions = int;
namespace SearchOption {

    /**
     * 若当前作用域没有目标，就到当前作用域所在的作用域搜索
     * 对实现来说，将会跳转到实现对应的定义所在的作用域 */
    constexpr SearchOptions PARENT = 0x0001;

    /**
     * 若当前作用域没有目标，就跳转到当前类的基类去搜索目标 */
    constexpr SearchOptions SUPER = 0x0002;

    /**
     * 若当前作用域没有目标，首先搜索基类，再搜索当前作用域的父作用域 */
    constexpr SearchOptions ALL = PARENT | SUPER;

    /**
     * 若当前作用域是类，搜索结果接受内部定义 */
    constexpr SearchOptions INNERS = 0x0100;

    /**
     * 若当前作用域是类，搜索结果接受成员 */
    constexpr SearchOptions MEMBERS = 0x0200;

    /**
     * 若当前作用域是类，搜索结果接受模板参数 */
    constexpr SearchOptions TARGS = 0x0400;

    /**
     * 若当前作用域是类，搜索结果接受任何 */
    constexpr SearchOptions ANY = INNERS | MEMBERS | TARGS;
}

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

        /**
         * @member dep_cache : 依赖关系缓冲 */
        map<$depdesc, $module> dep_cache;

        /**
         * @member searching_layers : 搜索层
         * @desc : 用于检查循环搜索的缓冲 */
        chainz<$scope> searching_layers;

        /**
         * @member alias_searching_layers : 别名搜索
         * @desc : 和searching_ayers 联合组成循环搜索检查数据基础 */
        chainz<$aliasdef> alias_searching_layers;

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
         * @method getModule : 获取模块
         * @desc :
         *  根据依赖描述符定位模块
         */
        $module getModule( $depdesc );

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
         * @param name : 要查询的名称
         * @param opts : 搜索选项
         * @param scope : 指定作用域，若此参数省略，则从名称表达式中提取作用域
         * @param paddings : 挂起别名，当搜索别名时，为了避免循环别名造成无限递归，要将正在搜索的别名挂起
         * @return everything : 查询结果，遭遇错误就返回空集
         *  诊断信息会沿着语法结构上传到模块诊断容器 */
        static everything Reach( $nameexpr name, SearchOptions opts, $scope scope);
        static everything $( $nameexpr name, SearchOptions opts = SearchOption::ANY|SearchOption::ALL, $scope scope = nullptr );

        /**
         * @static-method ReachClass : 尝试抵达一个类定义
         * @param name : 要查询的名称
         * @param opts : 搜索选项
         * @param scope : 指定作用域，若此参数省略，则从名称表达式中提取作用域
         * @param paddings : 挂起别名，当搜索别名时，为了避免循环别名造成无限递归，要将正在搜索的别名挂起
         * @return everything : 查询结果，遭遇错误就返回空集
         *  诊断信息会沿着语法结构上传到模块诊断容器 */
        static $classdef ReachClass( $nameexpr name, SearchOptions opts = SearchOption::ANY|SearchOption::ALL, $scope scope = nullptr );

        /**
         * @static-method ReductPrototype : 归约一个元素原型
         * @desc :
         *  从上下文语境确定元素原型,若解析失败，会将数据类型设置为Unsolvable并返回空代理
         *  注: UnknownType 不能被解析，但不会被视为错误
         * @param proto : 元素原型
         * @return : 此方法会修改元素原型抽象语法结构的内容，并返回其自身引用
         */
        static $eprototype ReductPrototype( $eprototype proto );
        static $eprototype $( $eprototype proto );

        /**
         * @static-method ReductTypeexpr : 归约一个类型表达式
         * @desc :
         *  尝试从上下文语境推算数据类型，将未确定的数据类型归约成为确定的数据类型
         *  在NamedType指向了模板参数的情况下，若给出了元素原型的代理，此方法使用模板参数的元素类型替换元素原型中的元素类型
         * @param type : 要推算的数据类型
         * @param proto : 数据类型所属的元素原型
         * @return : 若推算成功返回数据类型自身的引用，若推算失败则返回空代理
         */
        static $typeexpr ReductTypeexpr( $typeexpr type, $eprototype proto );
        static $typeexpr $( $typeexpr type, $eprototype proto = nullptr);
        
        /**
         * @static-method GetDefinition : 获取实现对应的定义 */
        static $definition GetDefinition( $implementation );

        /**
         * @static-method GetThisClassDef : 获取语句所在的实现的当前类定义 */
        static $classdef GetThisClassDef( $node n );

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

        /**
         * @method GetTemplateUsage : 获取模板用例
         * @desc :
         *  对模板类，传入模板参数获取用例
         *  若出现错误，则向def所属的语义上下文填写诊断信息
         * @param def : 模板类定义
         * @param targs : 模板参数列表
         * @return $classdef : 若成功返回用例类定义，若失败，返回空
         */
        static $classdef GetTemplateUsage( $classdef def, eprototypes targs );

        /**
         * @method IsIdentical : 判断元素原型是否完全一致
         * @desc :
         *  判断两个元素原型是否完全一致
         *  元素原型一致的条件是，元素类型一致，约束一致，数据类型一致
         * @param a : 第一个元素原型
         * @param b : 第二个元素原型
         * @param u : 未知数据类型是否判定为相同
         * @return bool : 若元素原型一致则返回true
         */
        static bool IsIdentical( $eprototype a, $eprototype b, bool u = false );

        /**
         * @method IsIdentical : 判断数据类型是否一致
         * @desc :
         *  判断两个数据类型是否一致
         *  不能解析的数据类型不一致
         *  未知数据类型与任何数据类型不一致，包括未知数据类型
         *  参数u所代表的语义不能在callable_type的参数和返回原形之间传递
         *  函数内部递归检查callable_type是否相同时，会认为同一位置的两个参数或返回原形，若都是Unknown则判定为相同
         * @param a : 第一个数据类型
         * @param b : 第二个数据类型
         * @param u : 未知数据类型是否判定为相同
         * @return bool : 若数据类型一致则返回true
         */
        static bool IsIdentical( $typeexpr a, $typeexpr b, bool u = false );

        /**
         * @method GetInheritTable : 获取继承表
         * @desc :
         *  通过Reach方法获取继承关系树，若存在循环继承则自动忽略
         * @param def : 要获取继承表的定义
         * @param paddings : 用于防止无限递归的挂载表
         * @return classdefs : 按照书写顺序排列的继承关系
         */
        static classdefs GetInheritTable( $classdef def, classdefs paddings = {} );

    protected:
        class searching_layer {
            private:
                chainz<$scope>* layers;
            public:
                searching_layer( $scope scope, $nameexpr name);
                ~searching_layer();
                operator bool()const;
        };
};

}

#endif