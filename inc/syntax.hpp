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

class CompilerContext;

/** used to declare syntax structures */
#define SYNTAX(x) struct x; using $##x = agent<x>
#define SYNTAXS(x) SYNTAX(x); using x##s = chainz<$##x>
#define SYNTAXES(x) SYNTAX(x); using x##es = chainz<$##x>

SYNTAX(node);

/** inherit from node */
SYNTAXS(signature);
SYNTAXS(depdesc);
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
SYNTAXS(blockstmt);     //block statement 代码块
SYNTAXS(element);       //element 元素语句
SYNTAXS(fctrlstmt);     //flow control statement 流控制语句
SYNTAXS(exprstmt);      //expression statement 表达式语句
SYNTAXS(branchstmt);    //branch statement 分支语句
SYNTAXS(switchstmt);    //switch statement 选择语句
SYNTAXS(loopstmt);      //loop statement 循环语句
SYNTAXS(assumestmt);    //assume statement 类型推断语句
SYNTAXS(dostmt);        //do statement 展开执行流/设置返回过程
SYNTAX(trystmt);
SYNTAX(catchstmt);
SYNTAX(throwstmt);

/**inherit from expr */
SYNTAXES(constant);     //constant expression 数值表达式,此类型表达式不一定能在编译期间取值
SYNTAXES(nameexpr);     //name expression 名称表达式
SYNTAXES(typeexpr);     //type expression 类型表达式
SYNTAXES(monoexpr);     //mono expression 单目运算表达式
SYNTAXES(binexpr);      //binary expression 双目运算表达式
SYNTAXES(callexpr);     //call expression 调用表达式
SYNTAXES(lambdaexpr);   //lambda expression lambda表达式
SYNTAXES(sctorexpr);    //structural constructing expression 结构构造表达式
SYNTAXES(lctorexpr);    //list constructing expression 列表构造表达式
SYNTAXES(tctorexpr);    //tuple constructing expression 元祖构造表达式
SYNTAXES(newexpr);      //new expression 在堆空间中建立对象
SYNTAXES(delexpr);      //delete expression 删除堆空间的对象
SYNTAXES(doexpr);       //do expression 同步执行
SYNTAXES(tconvexpr);    //type convert expr 类型转换表达式
SYNTAXES(aspectexpr);   //aspect expression 切面表达式
SYNTAXES(mbrexpr);      //member expression 成员表达式

/** module 并不是语法结构，而是语义结构，抽象层次比语法树节点要高一层 */
struct module;
using $module = agent<module>;
using modules = chainz<$module>;

using $scope = node*;

struct callable; using $callable = agent<callable>;
struct callable_type; using $callable_type = agent<callable_type>;

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

            BLOCKSTMT,          // 块语句
            ELEMENT,
            FCTRLSTMT,
            BRANCHSTMT,
            SWITCHSTMT,
            LOOPSTMT,
            ASSUMESTMT,
            DOSTMT,
            TRYSTMT,
            CATCHSTMT,
            THROWSTMT,

            EXPRESSION,         // 表达式语句
            CONSTANT,
            NAMEEXPR,           // 名称表达式
            TYPEEXPR,           // 类型表达式
            SCTOREXPR,
            LCTOREXPR,
            BINEXPR,
            MONOEXPR,
            LAMBDAEXPR,
            CALLEXPR,
            TCTOREXPR,
            NEWEXPR,
            DELEXPR,
            DOEXPR,
            TCONVEXPR,
            ASPECTEXPR,
            MBREXPR,
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
         * @method clone : 向新的作用域拷贝一份当前语法结构 
         * @desc :
         *  此方法只产生新的语法结构，不会将新的语法结构挂接在新的作用域
         * @param scope : 新的作用域
         * @return $node : 新的语法结构
         */
        virtual $node clone( $scope scope ) const = 0;

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

        /**
         * @method getCompilerContext : 获取编译器上下文
         * @desc :
         *  编译器上下文用于提供相关联资源的集中访问接口 */
        virtual CompilerContext& getCompilerContext();
};

/**
 * @struct callable_type : 可调用类型
 * @desc : 可调用类型描述了源代码层面上调用一个方法所需的所有信息 */
struct callable_type : public thing {

    public:
        /**
         * @member ret_proto: 返回值元素原型 */
        $eprototype ret_proto;

        /**
         * @member arg_protos: 参数的元素原型 */
        eprototypes arg_protos;

        /**
         * @member va_arg: 可变参数 
         * @desc: 若无效表示方法不使用可变参数；若为LABEL则使用静态可变参数；若为ETC则使用动态可变参数 */
        token va_arg;
};

/**
 * @struct callable : 可调用对象
 * @desc : 可调用对象对应于可调用类型 */
struct callable {

    public:
        /**
         * @member ret_proto: 返回值元素原型 */
        $eprototype ret_proto;

        /**
         * @member arguments: 参数的元素原型 */
        elements arguments;

        /**
         * @member va_arg: 可变参数 
         * @desc: 若无效表示方法不使用可变参数；若为LABEL则使用静态可变参数；若为ETC则使用动态可变参数 */
        token va_arg;

    public:
        $callable_type getType() const;
};

/**
 * @struct metprototype : 方法原型
 * @desc :
 *  方法原型包含了方法定义，方法实现都需要的内容，这些信息足已生成底层函数类型 */
struct metprototype : public callable {

    public:
        
        /**
         * @member cons : 约束
         * @desc : 是否约束此方法的this为const ref */
        token cons;

        /**
         * @member mode : 执行模式
         * @desc : 执行模式包括default,atomic,async,inline */
        token mode;

        /**
         * @member meta : 元方法
         * @desc : 元方法没有this引用，元方法是作用于类的方法，而不是作用在类实例上的方法 */
        token meta;
};

/**
 * @struct opprototype : 运算符原型
 * @desc : 运算符原型描述了产生底层运算符实现的入口所需的所有信息
 */
struct opprototype : public callable {

    public:
        /**
         * @member modifier : 修饰符
         * @desc : rev,ism,suffix,prefix,delete,default中的一个 */
        token modifier;

        /**
         * @member cons : 约束
         * @desc : 是否约束此运算符的this为const ref */
        token cons;

        /**
         * @member subtitle : 副标题
         * @desc : 某些特化运算符拥有副标题 */
        token subtitle;
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
         * @struct record : 语法树记录
         */
        struct record {
            enum status_t {
                failed = -1,
                unloaded,
                loaded,
            };

            /** @member status : 语法树状态 */
            status_t status = unloaded;

            /** @member ds : 诊断信息 */
            Diagnostics ds = {};
            
            /** @member fg : 语法树 */ 
            $fragment fg = nullptr;
        };
    
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
        map<fulldesc,record> docs;

        /**
         * @member space : 空间
         * @desc : 描述模块所在的空间，此值由编译器类负责填写，用于在补全依赖关系时提取信息 */
        srcdesc space;

        /**
         * @member context : 编译器上下文
         * @desc :　编译器上下文是直接管理模块签名的单位 */
        CompilerContext* context = nullptr;
    public:

        CompilerContext& getCompilerContext()override;
        virtual ~signature() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
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

        /**
         * @member doc : 文档来源
         * @desc : 描述此依赖描述符的来源文档 */
        srcdesc doc;

    public:
        virtual ~depdesc() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;

        /**
         * @method toJson : 转换成json格式存储
         * @desc :
         *  此过程会剔除主空间信息 */
        json toJson()const;
        static $depdesc fromJson( const json& object, srcdesc space );
        Uri getDocUri() override;
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
         * @desc : 将片段挂载进入编译器上下文时，由上下文负责填写 */
        srcdesc doc;

        /**
         * @member context : 编译器上下文
         * @desc : 将片段挂载进入编译器上下文时，由上下文负责填写 */
        CompilerContext* context = nullptr;

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
        $node clone( $scope scope ) const override;
        this_is_scope

        virtual Uri getDocUri() override;
        virtual $fragment getFragment() override;
        virtual CompilerContext& getCompilerContext() override;
};

/**
 * @struct defiition : 定义
 * @desc :
 *  Alioth中所有的定义语法结构都继承自此类
 *  类定义，枚举定义，运算符定义，方法定义，属性定义
 */
struct definition : virtual public node {

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
 * @struct implementation : 实现
 * @desc :
 *  实现包括方法实现和运算符实现
 */
struct implementation : public node {

    public:

        /**
         * @member host : 宿主
         * @desc : 实现所属的宿主类 */
        $nameexpr host;

        /**
         * @member name : 实现名称
         * @desc : 实现名称主要用于语法树检索 */
        token name;

        /**
         * @member body : 方法实现体
         * @desc : 方法实现体是一个块语句 */
        $blockstmt body;
};

/**
 * @struct statement : 语句
 * @desc : 所有执行块以及执行块的内容都被称为语句
 */
struct statement : virtual public node {

    public:

    /**
     * @member name : 语句名称
     * @desc : 语句名称即语句的标签，语法上，并不是所有的语句都拥有名称 */
    token name;
};

/**
 * @struct eprototype : 元素原型
 * @desc :
 *  用于描述联系表达一个元素原型的语法结构
 */
struct eprototype : virtual public node {

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

        /**
         * @member cons : 元素约束
         * @desc : 
         *  对obj,ptr约束直接绑定的对象不可变
         *  对ref,rel约束间接绑定的对象不可变 */
        token cons;

    public:
        
        virtual ~eprototype() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
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
         * @member defs : 定义内容
         * @desc : 类定义的内容定义都包含于此，包括子类，运算符，属性定义，方法定义，别名定义 */
        definitions defs;

        /**
         * @member usages : 用例
         * @desc : 模板类用例存储在模板类的usages容器中 */
        classdefs usages;

    public:

        virtual ~classdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
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
        $nameexpr target;

    public:

        virtual ~aliasdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
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
        constantes items;

    public:

        virtual ~enumdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct attrdef : 属性定义
 * @desc :
 *  属性定义语法结构
 */
struct attrdef : public definition {

    public:

        /**
         * @member proto : 元素原型
         * @desc : 成员的元素原型 */
        $eprototype proto;

        /**
         * @member arr : 数组
         * @desc : 描述元素是否成为一个数组,存储顺序同书写顺序 */
        chainz<int> arr;

        /**
         * @member meta : 元
         * @desc : 元属性是类本身的属性，不属于类实例 */
        token meta;

    public:
        virtual ~attrdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct opdef : 运算符定义
 * @desc : 
 *  运算符定义包含默认运算符，删除默认运算符，和重载运算符 */
struct opdef : public definition, public opprototype {

    public:
        virtual ~opdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct metdef : 方法定义
 * @desc :
 *  方法定义语法结构
 */
struct metdef : public definition, public metprototype {

    public:

        /**
         * @member raw : 原始符号
         * @desc : 若无效，使用Alioth规则产生底层符号，若为raw约定，则使用方法名产生底层符号，若为字面字符串，则使用字符串内容产生底层符号 */
        token raw;

    public:
        virtual ~metdef() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct opimpl : 运算符实现
 * @desc : 运算符实现 */
struct opimpl : public implementation, public opprototype {

    public:

        /**
         * @member subtitle : 副标题 */
        token subtitle;

        /**
         * @member supers: 基类构造语句 */
        exprstmts supers;

        /**
         * @member initiate : 成员初始化列表
         * @desc : 利用表达式的name属性表示成员名 */
        exprstmts initiate;

    public:
        virtual ~opimpl() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct metimpl : 方法实现
 * @desc : 方法实现 */
struct metimpl : public implementation, public metprototype {

    public:
        virtual ~metimpl() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct blockstmt : 块语句
 * @desc :
 *  块语句由大括号包含，内容是语句序列 */
struct blockstmt : public statement, public statements {

    public:
        virtual ~blockstmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct element : 元素语句
 * @desc :
 *  元素语句用于创建一个元素绑定一个即将创建的对象
 *  元素语句使用name属性表示元素名称 */
struct element : public statement {

    public:

        /**
         * @member proto : 元素原型 */
        $eprototype proto;

        /**
         * @member init : 初始化表达式
         * @desc : 对元素数组来说，必须是列构造表达式 */
        $exprstmt init;

        /**
         * @member array : 数组
         * @desc : -1表示暂时省略了数组长度 */
        chainz<int> array;

    public:
        virtual ~element() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct fctrlstmt : 流控制语句
 * @desc : 
 *  流控制语句包括break,continue,return,break! 
 *  流控制语句使用name属性表示可能存在的目标标签 
 */
struct fctrlstmt : public statement {

    public:

        /**
         * @member action : 控制动作 */
        token action;

        /**
         * @member expr : 流控制语句可能携带的表达式 */
        $exprstmt expr;
    
    public:
        virtual ~fctrlstmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct exprstmt : 表达式语句
 * @desc :
 *  表达式语句表示任何表达式，包括名称表达式，类型表达式，值表达式，单目运算表达式，双目运算表达式，调用表达式，lambda表达式，若干种构造表达式
 */
struct exprstmt : public statement {

    public:
        enum expr_t {
            name_expr,
            type_expr,
            constant, // constant value

            negative, // - expr
            lnot, // not expr
            brev, // ~ expr
            address, // & expr
            refer, // * expr
            preinc, // ++expr
            predec, // --expr
            
            postinc, // expr++
            postdec, // expr--
            index, // expr[expr]

            add, // expr + expr
            sub, // expr - expr
            mul, // expr * expr
            div, // expr / expr
            mol, // expr % expr

            shl, // expr << expr
            shr, // expr >> expr
            bxor, // expr ^ expr
            bor, // expr | expr
            band, // expr & expr

            land, // expr and expr
            lor, // expr or expr
            lxor, // expr xor expr

            eq, // expr == expr
            ne, // expr != expr
            ge, // expr >= expr
            le, // expr <= expr
            gt, // expr > expr
            lt, // expr < expr

            pointer, // expr->token
            member, // expr . token
            treat, // expr as! typeexpr
            as, // expr as typeexpr
            aspect, // expr # token

            assign, // expr = expr
            addass, // expr += expr
            subass, // expr -= expr
            mulass, // expr *= expr
            divass, // expr /= expr
            molass, // expr %= expr
            shlass, // expr <<= expr
            shrass, // expr >>= expr
            bxorass, // expr ^= expr
            borass, // expr |= expr
            bandass, // expr &= expr

            lambda, // $(...) ... {...}
            sctor, // {...}
            lctor, // [...]
            tctor, // <...>
            call, // expr(...=>...)
            newexpr, // new type expr
            delexpr, // delete expr
            doexpr, // do expr
        };
    
    public:
        expr_t etype;

    public:
        virtual ~exprstmt() = default;
};

/**
 * @struct branchstmt : 分支语句
 * @desc : 分支语句即if-else语句 */
struct branchstmt : public statement {

    public:
        /**
         * @member condition : 分支条件 */
        $exprstmt condition;

        /**
         * @member branch_true : 条件成立的分支 */
        $statement branch_true;

        /**
         * @member branch_false : 条件不成立的分支 */
        $statement branch_false;

    public:
        virtual ~branchstmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct switchstmt : 选择语句
 * @desc : 
 *  选择语句可以使用name属性作为标签，以便被流控制语句寻址
 */
struct switchstmt : public statement {

    public:

        /**
         * @member argument : 测试表达式 */
        $exprstmt argument;

        /**
         * @member branchs : 分支序列
         * @desc : 每个分支被自动识别为语句块，每个分支的引导常量即此分支的name属性 */
        blockstmts branchs;

        /**
         * @member defbr : 默认分支
         * @desc : 使用default打开的分支 */
        $blockstmt defbr;
    
    public:
        virtual ~switchstmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct loopstmt : 循环语句
 * @desc :
 *  循环语句可以使用name属性作为标签，以便被流控制语句寻址
 */
struct loopstmt : public statement {

    public:

        /** @enum loop_type : 循环类型 */
        enum loop_type {
            infinite, // uses body
            condition, // uses body con
            suffixed, // uses body con
            step, // uses body it con ctrl
            iterate, // uses body it con
            keyvalue, // uses body it con key
        };
    public:

        /**
         * @member ltype : 循环类型
         * @desc : 决定了哪些成员有效 */
        loop_type ltype;

        /**
         * @member con : 条件/容器 */
        $exprstmt con;

        /**
         * @member key : 键 */
        $element key;

        /**
         * @member it : 初始化语句/迭代器因子 */
        $statement it;

        /**
         * @member ctrl : 控制表达式 */
        $exprstmt ctrl;

        /** 
         * @member body : 循环体 */
        $statement body;
    
    public:
        virtual ~loopstmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct assumestmt : 假设语句
 * @desc :
 *  assume expr is prototype statement
 *  otherwise statement
 */
struct assumestmt : public statement {

    public:

        /**
         * @member expr : 假设测试表达式 */
        $exprstmt expr;

        /**
         * @member variable : 假设的目标原型，和假设成立时产生的变量 */
        $element variable;

        /**
         * @member branch_true : 假设成立分支 */
        $statement branch_true;

        /**
         * @member branch_false : 假设不成立分支
         * @desc : 假设不成立的分支所属的作用域是假设语句所属的作用域
         * 如此便可以在名称搜索时，绕开没能成功推断类型的变量 */
        $statement branch_false;

    public:
        virtual ~assumestmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct dostmt : DO语句
 * @desc :
 *  do语句可用于两种格式
 *      do statement when return;
 *      do expression [on expression] then expression;
 */
struct dostmt : public statement {

    public:

        /**
         * @member when : 语句目的
         * @desc : 若有效,表示do statement when return格式 */
        token when;

        /**
         * @member task : 任务
         * @desc : 对when来说，是一个块语句，对then来说，必须是一个可调用的表达式 */
        $statement task;

        /**
         * @member on : 数据集
         * @desc : 执行流展开时所使用的数据集，必须满足迭代约定 */
        $exprstmt on;

        /**
         * @member then : 回调任务
         * @desc : 必须是一个参数类型正确的可执行类型 */
        $exprstmt then;

    public:
        virtual ~dostmt() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct constant : 常量表达式
 * @desc : 常量表达式用于表示编译期就能推断数值的内容
 */
struct constant : public exprstmt {

    public:

        /**
         * @member value : 表达数值的记号 */
        token value;

    public:
        virtual ~constant() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
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
        $node clone( $scope scope ) const override;
};

/**
 * @struct typeexpr : 类型表达式
 * @desc :
 *  类型表达式的前身就是Alioth 0.2.0中的typeuc，以前被称为类型用例
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
         *  StructType : classdef
         *  EntityType : classdef
         *  CallableTyped : callable_type
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
        $node clone( $scope scope ) const override;
        bool is_type( typeid_t )const;

        static $typeexpr unknown( $scope scope, token phrase );
};

/**
 * @struct monoexpr : 单目运算表达式
 * @desc : 单目运算表达式
 */
struct monoexpr : public exprstmt {

    public:
        $exprstmt operand;
    
    public:
        virtual ~monoexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct binexpr : 双目运算表达式
 * @desc : 双目运算表达式 */
struct binexpr : public exprstmt {

    public:
        /**
         * @member left : 运算符左侧的运算子 */
        $exprstmt left;

        /**
         * @member right : 运算符右侧的运算子 */
        $exprstmt right;

    public:
        virtual ~binexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct callexpr : 调用表达式
 * @desc : 调用一个可调用的对象 */
struct callexpr : public exprstmt {

    public:

        /**
         * @member callee: 被调对象 */
        $exprstmt callee;

        /**
         * @member params: 参数列表 */
        exprstmts params;

        /**
         * @member rproro: 返回原型指示器
         * @desc : 可选的，可以指定返回原形，以从众多同名同参数列表的方法中选择具体调用 */
        $eprototype rproto;

    public:
        virtual ~callexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct lambdaexpr : lambda表达式
 * @desc : lambda表达式产生动态的可调用对象 */
struct lambdaexpr : public exprstmt, public callable {

    public:

        /**
         * @member body : 执行体 */
        $blockstmt body;

    public:
        virtual ~lambdaexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
        this_is_scope
};

/**
 * @struct sctorexpt : 结构化构造表达式
 * @desc :
 *  结构化构造表达式用于调用
 */
struct sctorexpr : public exprstmt, public exprstmts {

    public:
        /**
         * @member type_indicator : 类型指示器
         * @desc : 可选的可以为结构化构造表达式设置类型指示器 */
        $nameexpr type_indicator;
        
    public:
        virtual ~sctorexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct lctorexpr : 列构造表达式
 * @desc : 列构造表达式
 */
struct lctorexpr : public exprstmt, public exprstmts {
    public:
        /**
         * @member type_indicator : 类型指示器
         * @desc : 可选的可以为结构化构造表达式设置类型指示器 */
        $nameexpr type_indicator;

    public:
        virtual ~lctorexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct tctorexpr : 元祖构造表达式
 * @desc : 构造元祖的表达式
 */
struct tctorexpr : public exprstmt, public exprstmts {

    public:
        virtual ~tctorexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct newexpr : new表达式
 * @desc : 用于在堆空间中创建对象或数组 */
struct newexpr : public exprstmt {

    public:

        /**
         * @member ntype : 对象或数组的数据类型
         * @desc : 堆空间只能用于直接管理对象或指针，所以不需要声明元素类型 */
        $typeexpr ntype;

        /**
         * @member array : 数组长度
         * @desc : 在堆空间创建数组，数组长度可变，所以数组长度是表达式 */
        $exprstmt array;

        /**
         * @member init : 初始化表达式 */
        $exprstmt init;

    public:
        virtual ~newexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct delexpr : 删除表达式
 * @desc : 用于释放对象或数组占用的堆空间 */
struct delexpr : public exprstmt {

    public:

        /**
         * @member target : 要释放的对象 */
        $exprstmt target;

        /**
         * @member array : 是否正在释放一个数组空间 */
        token array;

    public:
        virtual ~delexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct doexpr : 同步表达式
 * @desc : 同步表达式使得执行流等待任务执行完毕 */
struct doexpr : public exprstmt {

    public:
        /**
         * @member task : 任务
         * @desc : 任务表达式必须是一个调用表达式 */
        $callexpr task;

    public:
        virtual ~doexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct tconvexpr : 类型转换表达式
 * @desc : 存在as和as!两种风格的类型转换 */
struct tconvexpr : public exprstmt {

    public:

        /**
         * @member org : 原表达式 */
        $exprstmt org;

        /**
         * @member proto : 目标原型 */
        $eprototype proto;

    public:
        virtual ~tconvexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct aspectexpr : 切面表达式
 * @desc : 切面表达式可以用于访问反射属性，也可用于访问静态索引成员 */
struct aspectexpr : public exprstmt {

    public:

        /**
         * @member host : 宿主对象 */
        $exprstmt host;

        /**
         * @member title : 切面标题 */
        token title;

    public:
        virtual ~aspectexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
};

/**
 * @struct mbrexpr : 成员表达式
 * @desc : 存在.和->两种用于访问成员的运算符 */
struct mbrexpr : public exprstmt {

    public:

        /**
         * @member host : 宿主对象
         * @desc : 宿主对象 */
        $exprstmt host;

        /**
         * @member nav : 导航名称
         * @desc : 导航名称可以是多层次的名称表达式，其中的前缀用于选择成员所属的基类 */
        $nameexpr nav;

    public:
        virtual ~mbrexpr() = default;
        bool is( type )const override;
        $node clone( $scope scope ) const override;
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

        /**
         * @param not_care : 若true，则不在乎元素原型是否完全为空 */
        $eprototype constructElementPrototype( $scope, bool not_care );

        $aliasdef constructAliasDefinition( $scope scope );
        $classdef constructClassDefinition( $scope scope );
        $enumdef constructEnumerateDefinition( $scope scope );
        $attrdef constructAttributeDefinition( $scope scope );
        $opdef constructOperatorDefinition( $scope scope );
        $metdef constructMethodDefinition( $scope scope );

        $opimpl constructOperatorImplementation( $scope scope );
        $metimpl constructMethodImplementation( $scope scope );

        $blockstmt constructBlockStatement( $scope scope );

        /** 
         * @note : 不要求指定数据类型
         * @param autowire : 是否正在扫描自动注入元素，自动注入元素由on或->结束定义，不接受初始化表达式 */
        $element constructElementStatement( $scope scope, bool autowire );
        /** @param intuple : 是否正在扫描元组内的表达式，这将会把>视为结束符 */
        $exprstmt constructExpressionStatement( $scope scope, bool intuple = false );
        $branchstmt constructBranchStatement( $scope scope );
        $switchstmt constructSwitchStatement( $scope scope );
        $assumestmt constructAssumeStatement( $scope scope );
        $loopstmt constructLoopStatement( $scope scope );
        $fctrlstmt constructFlowControlStatement( $scope scope );
        $dostmt constructDoStatement( $scope scope );

        /** @param absorb : 是否吸收左尖括号为模板参数列表的开头符号 */
        $nameexpr constructNameExpression( $scope scope, bool absorb );

        /** @param absorb : 用于表明是否已经处于可以吸收'<'的语境，此值可能由于class关键字的引导而改变 */
        $typeexpr constructTypeExpression( $scope scope, bool absorb );
        $constant constructConstantExpression( $scope scope );
        $lctorexpr constructListConstructingExpression( $scope scope );
        $sctorexpr constructStructuralConstructingExpression( $scope scope );
        $tctorexpr constructTupleConstructingExpression( $scope scope );
        $lambdaexpr constructLambdaExpression( $scope scope );
        $newexpr constructNewExpression( $scope scope );
        $delexpr constructDeleteExpression( $scope scope );
        $doexpr constructDoExpression( $scope scope );

        bool constructOperatorLabel( $scope scope, token& subtitle, $eprototype& proto );

        /** @param defv : 是否接受默认参数 */
        bool constructParameterList( $scope scope, callable& ref, bool defv);

        /** 
         * @param block: 是否将'{'视为块语句的开端
         * @param ele : 是否接收元素语句 */
        $statement constructStatement( $scope scope, bool block, bool ele );

        static int prio(const token& op );
};

}

#endif