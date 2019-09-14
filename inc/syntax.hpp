#ifndef __syntax__
#define __syntax__

#include "token.hpp"
#include "agent.hpp"
#include "jsonz.hpp"
#include "space.hpp"
#include "diagnostic.hpp"

namespace alioth {

/** used to declare syntax structures */
#define SYNTAX(x) struct x; using $##x = agent<x>
#define SYNTAXS(x) SYNTAX(x); using x##s = chainz<$##x>
#define SYNTAXES(x) SYNTAX(x); using x##es = chainz<$##x>

SYNTAX(node);
SYNTAXS(depdesc);
SYNTAXS(signature);
SYNTAXS(fragment);
SYNTAX(module);
SYNTAXS(definition);
SYNTAXS(implementation);
using $scope = node*;

/**
 * @struct node : 节点
 * @desc :
 *  语法树节点，所有的语法树节点都应当基于此基类，以便泛型保存
 */
struct node : public thing {

    public:
        enum type {
            MODULE,             // 模块
            FRAGMENT,           // 片段
            SIGNATURE,          // 签名
            DEPDESC,            // 依赖
            DEFINITION,         // 定义
            IMPLEMENTATION,     // 实现
            STATEMENT,          // 语句

            CLASSDEF,           // 类定义
            ENUMDEF,            // 枚举定义
            METHODDEF,          // 方法定义
            OPERATORDEF,        // 运算符定义
            ATTRIBUTEDEF,       // 属性定义

            METHODIMPL,         // 方法实现
            OPERATORIMPL,       // 运算符实现

            EXPRESSION,         // 表达式语句
        };

    private:

        /**
         * @member mscope : 作用域
         * @desc : 此节点所在的作用域 */
        $scope mscope;

    protected:

        /**
         * @ctor : 构造函数
         * @desc :
         *  构造函数必须被调用，因为作用域必须被设置
         * @param sc : 作用域
         */
        explicit node( $scope sc = nullptr );

    protected:
        node( const node& ) = default;
        node( node&& ) = default;

        node& operator=( const node& ) = default;
        node& operator=( node&& ) = default;

        virtual ~node() = default;
    public:

        /**
         * @method is : 类型判断
         * @desc :
         *  判断语法结构的类型
         * @param type : 语法结构类型
         * @return bool : 语法结构类型与参数是否相符
         */
        virtual bool is( type type ) const = 0;

        /**
         * @method isscope : 判断是否为作用域
         * @desc :
         *  判断语法树节点是否能作为作用域被记录在子节点中
         * @return bool : 判断结果
         */
        virtual bool isscope()const = 0;

        /**
         * @method setScope : 设置作用域
         * @desc :
         *  此方法用于给语法树节点设置作用域，只有当语法树节点为null时才会成功
         * @param sc : 作用域
         * @return bool : 执行是否成功
         */
        virtual bool setScope( $scope sc )final;

        /**
         * @method getScope : 获取作用域
         * @desc :
         *  此方法用于获取作用域
         * @return $scope : 此节点所在的作用域
         */
        virtual $scope getScope()const final;

        /**
         * @method getDocUri : 获取文档URI
         * @desc :
         *  获取此语法树节点所在的文档的URI,一般用于产生日志
         * @return Uri : 获取的结果
         */
        virtual Uri getDocUri();

        /**
         * @method getFragment : 获取片段
         * @desc :
         *  获取源码所在的片段
         * @return $fragment : 源码所在片段
         */
        virtual $fragment getFragment();

        /**
         * @method getModule : 获取模块
         * @desc :
         *  获取此节点所在的模块
         * @return $module : 模块
         */
        virtual $module getModule();
};

/**
 * @struct signature : 模块签名
 * @desc :
 *  每个片段必备的结构，用于在片段和模块，模块和模块建立联系
 */
struct signature : public node {
    
    public:
        /**
         * @member name : 模块名 */
        token name;

        /**
         * @member entry : 入口标记
         * @desc : 标记入口方法的名称，入口方法一定是透明类中的拥有特定原型的元方法 */
        token entry;

        /**
         * @member deps : 依赖集
         * @desc : 依赖描述符集合，模块对其他模块的依赖关系 */
        depdescs deps;

        /**
         * @member docs : 文档集
         * @desc : 此容器用于方便编译器类记录模块的源码来源不被语法分析器负责 */
        chainz<SpaceEngine::FullDesc> docs;

        /**
         * @member space : 空间
         * @desc : 描述模块所在的空间，此值由编译器类负责填写，用于在补全依赖关系时提取信息 */
        SpaceEngine::Desc space;

    public:

        virtual ~signature() = default;
        bool is( type )const override;
        bool isscope()const override;
};

/**
 * @struct depdesc : 依赖描述符
 * @desc :
 *  依赖描述符是用于描述一道依赖关系的抽象结构
 */
struct depdesc : public node {

    public:
        /**
         * @member name : 模块名
         * @desc : 以来的模块的名称 */
        token name;

        /**
         * @member from : 模块的来源包
         * @desc : 来源包细节
         *  若此域留白，按照本地空间->根空间的顺序搜索
         *  若书写形式为'.'则表示本地空间
         *  若书写形式为'alioth'则表示根空间
         *  若书写形式为其他内容，被视为包名，其中'^'字符及其后的内容会被截断，视为包版本限制 */
        token from;

        /**
         * @member alias : 别名
         * @desc : 若别名设置为this module, 内部会使用 */
        token alias;

    public:
        virtual ~depdesc() = default;
        bool is( type )const override;
        bool isscope()const override;
        json toJson()const;
};

/**
 * @struct fragment : 片段
 * @desc :
 *  从源文档直接建立的语法树不能描述完整的模块信息，所以称之为片段
 */
struct fragment : public node {

    public:

        /**
         * @member source : 源码文档
         * @desc : 此值应当由编译器类负责填写 */
        SpaceEngine::Desc source;

        /**
         * @member defs : 定义
         * @desc : 源文档中所有的定义 */
        definitions defs;

        /**
         * @member impls : 实现
         * @desc : 源文档中的所有实现 */
        implementations impls;
    public:

        virtual ~fragment() = default;
        bool is( type )const override;
        bool isscope()const override;
};

/**
 * @class SyntaxContext : 语法上下文
 * @desc :
 *  语法分析上下文，语法分析任务可以被认为是语法分析上下文上的一系列操作
 *  描述一个用于实现LR文法分析算法的有限状态机
 *  此状态机面向状态归约,简化文法分析程序的设计
 */
class SyntaxContext {

    private:
    
        /**
         * @struct state : 状态
         * @desc :
         *  描述一个有限状态机的状态
         */
        struct state {

            public:

                /**
                 * @member s : 状态名
                 * @desc :
                 *  用于实现基于状态转换的语法分析过程 */
                int s;

                /**
                 * @member c : 符号统计
                 * @desc :
                 *  记载当前状态下保留了多少文法符号 */
                int c;

            public:

                /**
                 * @method state : 构造方法
                 * @desc : 构造一个状态就意味着进入了一个新的状态
                 *      此动作的语义是,移进c个单词,进入s状态
                 * @param _s : 新状态的状态名
                 * @param _c : 新状态初始状态下挂起的单词总量
                 */
                state(int _s,int _c = 1);

                /**
                 * @operator int& : 类型转换方法
                 * @desc : 将state当成一个数字使用时,state的语义就代表它自己的状态名称
                 */
                operator int&();
        };

        /**
         * @struct wb : 工作台
         * @desc :
         *  借助析构函数帮助工作流清理环境
         */
        struct wb {
            wb( int level, int work, SyntaxContext& context ):
                l(level),w(work),c(context){}
            ~wb(){ 
                if( auto s = c.states.size(); w > 0 and l > 0 and w == c.ws.size() and s > l ) {
                    c.movo(s-l); c.working(); }}
            wb( wb&& b ):l(b.l),w(b.w),c(b.c) {
                b.l = b.w = -1; }
            wb(const wb& ) = delete;

            private:
                int l;    // 状态栈层次
                int w;    // 工作流层次
                SyntaxContext& c; // 上下文
        };
        friend struct wb;

    private:

        /**
         * @member source : 源码记号序列 */
        tokens& source;

        /**
         * @member diagnostics : 日志序列 */
        Diagnostics& diagnostics;

        /**
         * @member states : 状态栈
         * @desc :
         *  状态栈中的状态序列
         *  描述了归约,移出动作所需的信息,跟踪记录有限状态机的状态变化轨迹
         *  如此,归约动作便不需要明确指向某个目标状态,而是明确归约的状态个数,按照顺序弹出状态栈即可
         *  继而,归约动作在局部范围内的功能便成为了可复用可重入的状态. */
        chainz<state> states;

        /**
         * @member it : 终结符序列迭代器
         * @desc :
         *  进行归约,移进等操作时,有限状态机需要从迭代器输入
         *  记号,同时移动迭代器的输入指针 */
        tokens::iterator it;

        /**
         * @member ws : 工作流
         * @desc : 用于记录每个工作流程开始之前状态栈的层数，帮助工作流恢复状态栈*/
        chainz<int> ws;

    public:
        /**
         * @ctor : 构造函数
         * @desc :
         *  构造函数从token序列构造语法上下文，使用引用以减小内存开销。
         *  这意味着源token序列将在语法分析过程中被修改，如果你依然需要它
         *  请自行保存。
         * @param source : token序列引用
         * @param diagnostics : 诊断信息容器
         */
        SyntaxContext( tokens& source, Diagnostics& diagnostics );

        /** 
         * @method extractSignature : 提取模块签名
         * @desc :
         *  从源码中提取模块签名
         *  不负责填写docs,space内容
         * @param diagnostic : 是否产生日志
         * @return $signature : 签名
         */
        $signature extractSignature( bool diagnostic = true );

    private:
        /**
         * @method movi : 移进方法
         * @desc : 移进c个单词,并进入s状态
         *      c个单词被归属与s状态中挂起的单词
         * @param s : 新的状态名称
         * @param c : 移进单词的总量,通常是1
         */
        void movi(int s, int c=1);

        /**
         * @method movo : 移出方法
         * @desc : 用于退回c个移进动作
         *      主要用于在判定后决定撤销一个预动作时使用
         *      如果移出的状态中包含了归约动作,则很有可能造成混乱,需要谨慎使用
         * @param c : 要移出的状态个数
         */
        void movo(int c = 1 );

        /**
         * @method stay : 停滞方法
         * @desc : 用于将更多的单词挂载到当前状态中
         *      这有助于在一个状态中识别可选的重复的几个单词
         *      因为若单词的个数不确定,就不能在运行之前给状态确定名称
         * @param c : 要挂起的单词的个数
         */
        void stay(int c = 1 );

        /**
         * @method redu : 归约方法
         * @desc : 用于将几个状态下的单词全部归约成一个非终结符
         *      此动作也可能携带当前即将输入的一个单词
         * @param c : 要归约的状态个数
         *      c >= 0 --> 归约即将输入的单词,以及c个状态中挂起的单词
         *      c < 0 --> 仅归约-c个状态中挂起的单词
         * @param n : 归约后,非终结符的id
         */
        void redu(int c, int n );

        /**
         * @method enter : 进入
         * @desc :
         *  进入一个工作流程，记录当前状态栈层次
         * @return wb : 工作台，工作台析构时会确保状态栈恢复正常
         */
        wb enter();

        /**
         * @method working : 检查工作状态
         * @desc :
         *  若状态栈层次高于最后的记录，则认为工作流程尚未结束，返回true
         *  否则，自动删除最后一个工作状态，并返回false
         * @return bool : 是否继续工作
         */
        bool working();

    private:

        $signature constructModuleSignature( $scope scope, bool diagnostic = true );
        $depdesc constructDependencyDescriptor( $scope scope, bool diagnostic = true );
};

}

#endif