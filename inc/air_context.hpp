#ifndef __air_context__
#define __air_context__

#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>
#include "diagnostic.hpp"
#include "syntax.hpp"
#include "agent.hpp"

namespace alioth {

class SemanticContext;
class module;
using $module = agent<module>;

/** 用于快速判断方法签名特点的方法属性 */
using metattrs = int;
namespace metattr {

    /** 有this追加参数存在 */
    constexpr metattrs tsarg = 0x01;

    /** 没有普通参数 */
    constexpr metattrs noarg = 0x02;

    /** 启用可变参数 */
    constexpr metattrs vaarg = 0x04;

    /** 返回结构 */
    constexpr metattrs retst = 0x08;

    /** 返回运行时类型识别包 */
    constexpr metattrs retri = 0x10;

    /** 返回引用 */
    constexpr metattrs retrf = 0x20;

    /** [检查用]返回结构复杂 */
    constexpr metattrs retcm = retrf|retri|retst;
};

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

        /**
         * @member module : 当前正在翻译的模块 */
        std::shared_ptr<llvm::Module> module;

        /**
         * @member alioth : 内置模块 */
        std::shared_ptr<llvm::Module> alioth;

        /**
         * @member named_types : 具名类型表 */
        std::map<std::string,llvm::StructType*> named_types;

        /**
         * @member element_values : 具名元素表 */
        std::map<$element,llvm::Value*> element_values;

        /**
         * @member method_attrs : 方法属性 */
        std::map<$metdef,metattrs> method_attrs;

    public:

        /**
         * @ctor : 构造函数
         * @desc : 给定架构和平台，初始化LLVM上下文，并捕获诊断信息容器
         */
        AirContext( string arch, string platform, Diagnostics& diag );

        /**
         * @dtor : 析构函数
         * @desc : 销毁需要手动管理生命周期的对象
         */
        ~AirContext();

        /**
         * @operator () : 处理模块
         * @desc :
         *  将语义模块翻译为目标文件，输出至目标输出流
         */
        bool operator()( $module mod, ostream& os, bool ir = false );

        /**
         * @operator () : 处理模块
         * @desc :
         *  尝试将语义模块翻译为机器码，不写入文件
         */
        bool operator()( $module mod );

        /**
         * @operator () : 处理alioth模块
         * @desc :
         *  尝试将alioth模块翻译至机器码,输出至目标输出流 */
        bool operator()( ostream& os, bool ir = false );

    private:

        void initInlineStructures();

        bool translateModule( $module );
        bool translateDefinition( $definition );
        bool translateImplementation( $implementation );

        bool translateClassDefinition( $classdef );
        bool translateEnumDefinition( $enumdef );
        bool translateMethodDefinition( $metdef );
        bool translateOperatorDefinition( $opdef );

        bool translateMethodImplementation( $metimpl );
        bool translateOperatorImplementation( $opimpl );

        bool translateStatement( $statement stmt );
        bool translateBlockStatement( $blockstmt stmt );
        bool translateElementStatement( $element stmt );
        bool translateBranchStatement( $branchstmt stmt );
        bool translateSwitchStatement( $switchstmt stmt );
        bool translateAssumeStatement( $assumestmt stmt );
        bool translateLoopStatement( $loopstmt stmt );
        bool translateFlowControlStatement( $fctrlstmt stmt );
        bool translateDoStatement( $dostmt stmt );

        bool translateExpressionStatement( $exprstmt stmt );
        bool translateNameExpression( $nameexpr );
        bool translateConstantExpression( $constant );
        bool translateListConstructingExpression( $lctorexpr );
        bool translateStructuralConstructingExpression( $sctorexpr );
        bool translateTupleConstructingExpression( $tctorexpr );
        bool translateLambdaExpression( $lambdaexpr );
        bool translateNewExpression( $newexpr );
        bool translateDeleteExpression( $delexpr );
        bool translateDoExpression( $doexpr );

        /** 产生一个start函数作为入口,它将整理命令行参数，调用入口方法 */
        bool generateStartFunction( $metdef met );

        /** 产生输出入 */
        bool generateOutput( std::shared_ptr<llvm::Module> mod, ostream& os, bool ir );

        /**
         * @method $t : 获取类实例的类型 */
        llvm::StructType* $t( $classdef );

        /**
         * @method $et : 获取类实体的类型 */
        llvm::StructType* $et( $classdef );

        /**
         * @method $t : 获取属性类型 */
        llvm::Type* $t( $attrdef );

        /**
         * @method $t : 获取方法类型 */
        llvm::FunctionType* $t( $metdef );

        /**
         * @method $t : 获取运算符的函数类型 */
        llvm::FunctionType* $t( $opdef );

        /**
         * @member $t : 获取元素原型的类型 */
        llvm::Type* $t( $eprototype );

        /**
         * @member $t : 解算数据类型 */
        llvm::Type* $t( $typeexpr );

        /**
         * @member $a : 获取方法属性 */
        metattrs $a( $metdef );

        /**
         * @method $ : 获取具名类型
         * @desc :
         *  查询具名类型表，若存在返回类型指针
         *  若对应类型不存在，则先构建空的类型填入表中，再返回
         */
        llvm::StructType* $t( const std::string& );

        /**
         * @method $e : 获取类实体的全局变量
         * @desc :
         *  若尚未创建则插入
         */
        llvm::GlobalVariable* $e( $classdef def );

        /**
         * @method createGlobalStr : 创建全局字符串
         * @desc : 在alioth模块创建全局变量
         */
        llvm::GlobalVariable* createGlobalStr( const string& str );
};

}

#endif