#ifndef __syntax__
#define __syntax__

#include <set>
#include "type.hpp"
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

/** inherit from node */
SYNTAXS(depdesc);
SYNTAXS(signature);
SYNTAXS(fragment);
SYNTAXS(definition);
SYNTAXS(implementation);
SYNTAXS(statement);
SYNTAXS(eprototype);

/** inherit from definition **/
SYNTAXS(classdef);      //class definition 类定义
SYNTAXS(aliasdef);      //alias definition 别名定义
SYNTAXS(enumdef);       //enumerate definition 枚举定义
SYNTAXS(attrdef);       //attribute definition 属性定义
SYNTAXS(opdef);         //operator definition 运算符定义
SYNTAXS(metdef);        //method definition 方法定义

/** inherit from implementation */
SYNTAXS(opimpl);        //operator implementation 运算符实现
SYNTAXS(metimpl);       //method implementation 方法实现

/** inherit from statement */
SYNTAXES(blockstmt);    //block statement 代码块
SYNTAXES(exprstmt);     //expression statement 表达式语句
SYNTAXES(branchstmt);   //branch statement 分支语句
SYNTAXES(switchstmt);   //switch statement 选择语句
SYNTAXES(assumestmt);   //assume statement 类型推断语句
SYNTAXES(loopstmt);     //loop statement 循环语句
SYNTAXES(fctrlstmt);    //flow control statement 流控制语句
SYNTAXES(unwindstmt);   //unwind statement 执行流展开语句
SYNTAXES(doretstmt);    //do return statement 返回过程挂载语句

/**inherit from expr */
SYNTAXES(nameexpr);     //name expression 名称表达式
SYNTAXES(typeexpr);     //type expression 类型表达式
SYNTAXES(monoexpr);     //mono expression 单目运算表达式
SYNTAXES(binexpr);      //binary expression 双目运算表达式
SYNTAXES(valexpr);      //value expression 数值表达式
SYNTAXES(lctexpr);      //list constructing expression 列表构造表达式
SYNTAXES(sctexpr);      //structural constructing expression 结构构造表达式
SYNTAXES(tctexpr);      //tuple constructing expression 元祖构造表达式
SYNTAXES(callexpr);     //call expression 调用表达式
SYNTAXES(lambdaexpr);   //lambda expression lambda表达式

/** module 并不是语法结构，而是语义结构，抽象层次比语法树节点要高一层 */
struct module;
using $module = agent<module>;
using modules = chainz<$module>;

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
            ELEPROTO,           // 元素原型

            ALIASDEF,           // 别名定义
            CLASSDEF,           // 类定义
            ENUMDEF,            // 枚举定义
            METHODDEF,          // 方法定义
            OPERATORDEF,        // 运算符定义
            ATTRIBUTEDEF,       // 属性定义

            METHODIMPL,         // 方法实现
            OPERATORIMPL,       // 运算符实现

            EXPRESSION,         // 表达式语句
            NAMEEXPR,           // 名称表达式
            TYPEEXPR,           // 类型表达式
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
         *  默认返回false
         * @return bool : 判断结果
         */
        virtual bool isscope()const;
        #define this_is_scope bool isscope()const override{return true;}

        /**
         * @method setScope : 设置作用域
         * @desc :
         *  此方法用于给语法树节点设置作用域，只有当语法树节点为null时才会成功
         *  另一种特殊情况是当sc为空时，允许使用此方法清空作用域
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
 *  此结构应当被CompilerContext管理
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
         * @desc : 此容器用于方便编译器类记录模块的源码来源不被语法分析器负责 
         *  此容器中键和值都很重要，键用作编译器上下文管理文档的唯一句柄，值作为上下文管理片段的唯一句柄 */
        map<fulldesc,$fragment> docs;

        /**
         * @member space : 空间
         * @desc : 描述模块所在的空间，此值由编译器类负责填写，用于在补全依赖关系时提取信息 */
        srcdesc space;

    public:

        virtual ~signature() = default;
        bool is( type )const override;
        this_is_scope

        /**
         * @method toJson : 转化为json
         * @desc :
         *  将模块签名转化为json格式保存起来
         *  此过程或剥除文档的主空间信息
         *  此过程不保存space信息
         * @return json: json格式的模块签名
         */
        json toJson() const;
        /**
         * @method fromJson : 从json恢复模块签名
         * @desc :
         *  此方法用于从json恢复模块签名
         * @param object : json数据对象
         * @param space : 用于补全文档空间信息的space,也将被存储于签名中
         * @return $signature : 若恢复成功，则返回签名代理
         */
        static $signature fromJson( const json& object, srcdesc space );

        /**
         * @method combine : 合并签名
         * @desc :
         *  此方法用于将模块签名中的文档，依赖信息合并到当前签名中
         */
        bool combine( $signature );
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
        json toJson()const;
        static $depdesc fromJson( const json& object );
};

/**
 * @struct fragment : 片段
 * @desc :
 *  从源文档直接建立的语法树不能描述完整的模块信息，所以称之为片段
 */
struct fragment : public node {

    public:

        /**
         * @member doc : 源码文档
         * @desc : 此值应当由编译器类负责填写,条件允许时必须填写，此值是从fragment搜索对应signature的依据 */
        srcdesc doc;

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
        this_is_scope
};

/**
 * @struct eprototype : 元素原型
 * @desc :
 *  用于描述联系表达一个元素原型的语法结构
 */
struct eprototype : public node {

    public:
        /**
         * @enum type_t : 元素类型
         * @desc :
         *  描述元素类型的枚举 
         */
        enum type_t {
            var, // 自动元素类型
            obj, // 对象元素
            ptr, // 指针元素
            ref, // 引用元素
            rel, // 重载元素
        };

    public:
        /**
         * @member etype : 元素类型 */
        type_t etype;

        /**
         * @member dtype : 数据类型 */
        $typeexpr dtype;

    public:
        
        virtual ~eprototype() = default;
        bool is( type )const override;
};

/**
 * @struct defiition : 定义
 * @desc :
 *  Alioth中所有的定义语法结构都继承自此类
 *  类定义，枚举定义，运算符定义，方法定义，属性定义
 */
struct definition : public node {

    public:
        /**
         * @member name : 定义名称
         * @desc : 所有定义都有名称，命名规则都相同 */
        token name;

        /**
         * @member visibility : 可见性修饰
         * @desc : 可见性修饰词，可使用public,private,protected或+,-,* */
        token visibility;

        /**
         * @member premise : 定义前提
         * @desc : 实现此定义必备的前提条件，内容是谓词的下标，若此容器为空，表示不需要前提条件 */
        std::set<int> premise;
};

/**
 * @struct statement : 语句
 * @desc : 所有执行块以及执行块的内容都被称为语句
 */
struct statement : public node {

};

/**
 * @struct aliasdef : 别名定义
 * @desc :
 *  在定义作用域中，使用let关键字定义别名
 *  let <label> = <nameexpr>
 */
struct aliasdef : public definition {
    public:

        /**
         * @member target : 别名目标
         * @desc : 别名所指向的目标数据类型，
         * 虽然语法规定此语法规则用于给类定义制定别名，但在语义分析中，
         * 此结构单纯起到无差别转发功能，所以任何具名语法结构都可以定制别名 */
        $nameexpr tagret;

    public:

        virtual ~aliasdef() = default;
        bool is( type )const override;
};

/**
 * @struct classdef : 类定义
 * @desc :
 *  类定义所携带的信息包含类定义的内容定义，类的基类列表，类的模板参数列表
 *  高级类定义还包含类的谓词。模板类应当包含模板类用例
 */
struct classdef : public definition {

    public:

        /**
         * @struct predi : 谓词单元
         * @desc :
         *  谓词单元表达一个谓词条件
         */
        struct predi {

            /**
             * @member vn : 非终结符
             * @desc : 谓词的非终结符，用于在报错时提供文本坐标信息 */
            token vn;

            /**
             * @member targ : 模板参数
             * @desc : 谓词单元用于给模板参数添加限制，targ用于定位此谓词单元给哪个模板参数添加限制 */
            token targ;

            /**
             * @member rule : 谓词规则
             * @desc : 谓词单元所携带的规则
             *  0 : 无效
             *  1 : 元素类型不能为对象                      T != obj
             *  2 : 元素类型不能为指针                      T != ptr
             *  3 : 元素类型不能为引用                      T != ref
             *  4 : 元素类型不能为重载                      T != rel
             *  5 : 元素类型只能为对象                      T == obj
             *  6 : 元素类型只能为指针                      T == ptr
             *  7 : 元素类型只能为引用                      T == ref
             *  8 : 元素类型只能为重载                      T == rel
             *  9 : 数据类型不能为指针                      T >> obj
             *  10: 数据类型必须为指针                      T >> ptr
             *  11: 数据类型不能基于参数(参数不能为指针)       T <> (dtype) 
             */
            int rule;

            /**
             * @member type : 谓词参数
             * @desc : 谓词单元涉及数据类型判定时，可能需要一个类型表达式作为参数 */
            $typeexpr type;
        };

        /**
         * @alias predicate : 谓词
         * @desc :
         *  谓词由谓词单元构成，表达谓词单元之间取and关系
         */
        using predicate = chainz<predi>;

        /**
         * @alias predicates : 谓词列表
         * @desc :
         *  谓词列表用于表示谓词之间的or关系
         */
        using predicates = chainz<predicate>;

    public:

        /**
         * @member abstract : 抽象修饰符
         * @desc : 修饰此类是否为抽象类 */
        token abstract;

        /**
         * @member targf : 模板类模板参数列表形参
         * @desc : 模板类模板参数列表中个模板参数名称 */
        tokens targf;

        /**
         * @member preds : 谓词
         * @desc : 只有模板类拥有谓词，谓词由谓词表达式构成，谓词表达式由谓词单元构成 */
         predicates preds;

        /**
         * @member targs : 模板类模板参数列表实参
         * @desc : 当确定模板参数列表中每个模板参数的具体元素原型，将产生一个唯一的衍生类定义
         *  此类定义被称为模板类用例，此用例应当包含模板类实参信息 */
        eprototypes targs;

        /**
         * @member supers : 基类列表
         * @desc : 基类使用名称表达式保存，因为语法阶段无法分析基类是否可达 */
        nameexpres supers;

        /**
         * @member contents : 定义内容
         * @desc : 类定义的内容定义都包含于此，包括子类，运算符，属性定义，方法定义，别名定义 */
        definitions contents;

        /**
         * @member usages : 用例
         * @desc : 模板类用例存储在模板类的usages容器中 */
        classdefs usages;

    public:

        virtual ~classdef() = default;
        bool is( type )const override;
        this_is_scope
};

/**
 * @struct : 枚举定义
 * @desc : 枚举定义语法结构
 */
struct enumdef : public definition {

    public:
        /**
         * @member item : 枚举单元
         * @desc : 枚举单元的定义，顺序不能被打乱 */
        tokens items;

    public:

        virtual ~enumdef() = default;
        bool is( type )const override;
        this_is_scope
};

struct metdef : public definition {

};

struct opdef : public definition {

};

struct attrdef : public definition {

};

/**
 * @struct exprstmt : 表达式语句
 * @desc :
 *  表达式语句表示任何表达式，包括名称表达式，类型表达式，值表达式，单目运算表达式，双目运算表达式，调用表达式，lambda表达式，若干种构造表达式
 */
struct exprstmt : public statement {

};

/**
 * @struct nameexpr : 名称表达式
 * @desc :
 *  名称表达式用途广泛，可以用于引用任何具有名称的结构，也因此
 *  它并不一定总能被求值
 *  名称表达式可以使用作用域运算符进行链接，以此跨越定义域搜索具名结构
 */
struct nameexpr : public exprstmt {

    public:
        /**
         * @member name : 名称
         * @desc : 当前名称表达式所使用的名称 */
        token name;

        /**
         * @member targs : 模板参数列表
         * @desc : 模板参数列表实参，若引用模板类，必须传入模板参数，
         * 产生模板类用例，才能将其作为类定义使用
         * [思考]:既然这样，不如拒绝模板类成为类定义语法结构，
         *  而是定义新的关键字template引导一个和类定义很相似的模板定义 */
        eprototypes targs;

        /**
         * @member next : 下一级
         * @desc : 名称表达式中，由作用域运算符链接的，右侧的名称表达式是左侧名称表达式的next
         *  严格意义上来讲，next表达的是进入作用域内部，进行细化搜索，应当视为sub可能更合适 */
        $nameexpr next;

    public:
        virtual ~nameexpr() = default;
        bool is( type )const override;
};

/**
 * @struct typeexpr : 类型表达式
 * @desc :
 *  类型表达式的前身就是Alioth 0.2.0中的typeuc，以前被称为名称用例
 *  现在，我们认为类型表达式是表达式的一个子类型，它的特点是不总能被求值
 *  没错，有些情况下类型表达式也可能被求值，比如用于表示一个类的实体
 */
struct typeexpr : public exprstmt {

    public:

        /**
         * @member id : 类型ID
         * @desc : 类型ID可以用于表明类型的基本信息，并确定sub的语义 */
        typeid_t id;

        /**
         * @member sub : 类型子信息
         * @desc : 由类型ID决定其语义
         *  UnknownType : nullptr
         *  NamedType : nameexpr
         *  ThisClassType : nullptr
         *  UnsolvableType : nullptr
         *  CompositeType : classdef
         *  EntityType : classdef
         *  ConstraintedPointerType : typeexpr
         *  UnconstraintedPointerType : typeexpr
         *  NullPointerType : nullptr
         *  VoidType : nullptr
         *  BooleanType : nullptr
         *  Uint8Type : nullptr
         *  Uint16Type : nullptr
         *  Uint32Type : nullptr
         *  Uint64Type : nullptr
         *  Int8Type : nullptr
         *  Int16Type : nullptr
         *  Int32Type : nullptr
         *  Int64Type : nullptr
         *  Float32Type : nullptr
         *  Float64Type : nullptr */
        anything sub;

    public:

        virtual ~typeexpr() = default;
        bool is( type )const override;
        bool is_type( typeid_t )const;
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

        /**
         * @method constructFragment : 构建片段
         * @desc :
         *  从源代码中提取模块的片段信息
         * @return $fragment : 片段
         */
        $fragment constructFragment();
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

        $signature constructModuleSignature( bool diagnostic );
        $depdesc constructDependencyDescriptor( $scope scope, bool diagnostic );

        $eprototype constructElementPrototype( $scope );

        $aliasdef constructAliasDefinition( $scope scope );
        $classdef constructClassDefinition( $scope scope );
        $enumdef constructEnumerateDefinition( $scope scope );
        $attrdef constructAttributeDefinition( $scope scope );
        $opdef constructOperatorDefinition( $scope scope );
        $metdef constructMethodDefinition( $scope scope );

        $opimpl constructOperatorImplementation( $scope scope );
        $metimpl constructMethodImplementation( $scope scope );

        $blockstmt constructBlockStatement( $scope scope );
        $exprstmt constructExpressionStatement( $scope scope );
        $branchstmt constructBranchStatement( $scope scope );
        $switchstmt constructSwitchStatement( $scope scope );
        $assumestmt constructAssumeStatement( $scope scope );
        $loopstmt constructLoopStatement( $scope scope );
        $fctrlstmt constructFlowControlStatement( $scope scope );
        $unwindstmt constructUnwindStatement( $scope scope );
        $doretstmt constructDoReturnStatement( $scope scope );

        // @param absorb : 是否吸收左尖括号为模板参数列表的开头符号
        $nameexpr constructNameExpression( $scope scope, bool absorb );

        // @param absorb : 用于表明是否已经处于可以吸收'<'的语境，此值可能由于class关键字的引导二改变
        $typeexpr constructTypeExpression( $scope scope, bool absorb );
        $monoexpr constructMonoExpression( $scope scope );
        $binexpr constructBinaryExpression( $scope scope );
        $valexpr constructValueExpression( $scope scope );
        $lctexpr constructListConstructingExpression( $scope scope );
        $sctexpr constructStructuralConstructingExpression( $scope scope );
        $tctexpr constructTupleConstructingExpression( $scope scope );
        $callexpr constructCallExpression( $scope scope );
        $lambdaexpr constructLambdaExpression( $scope scope );
};

}

#endif