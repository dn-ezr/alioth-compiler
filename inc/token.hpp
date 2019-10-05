#ifndef __token__
#define __token__

#include "alioth.hpp"
#include "vt.hpp"
#include "vn.hpp"
#include <string>

namespace alioth {

class Diagnostics;

/**
 * @enum-class CT : 符号分类
 * @desc :
 *  符号分类是对文法符号的分类
 *  当需要以这些分类为单位判断文法符号时
 *  便可以简省代码复杂度,减小维护开销
 */
enum class CT {
    BASIC_TYPE,         //基础数据类型终结符
    ELETYPE,            //元素类型
    CONSTANT,           //字面常量
    ASSIGN,             //赋值运算符
    RELATION,           //关系运算符
    OPERATOR,           //所有运算符
    PREFIX,             //前缀运算符
    INFIX,              //中缀运算符
    SUFFIX,             //后缀运算符

    IMPLEMENTATION,     //表达式,分支,循环,流控制,块
    // OPL,                //运算符标签
    // OPL_SPECIAL,        //特化运算符标签
    // OPL_ASSIGN,         //赋值运算符标签
    // OPL_MONO,           //单目运算符标签
    // OPL_BINO,           //双目运算符标签

    /**
     * 约定标签
     * MF -- modifier
     * LB -- label
     */
    // MF_ABSTRACT,
    // MF_REV,
    // MF_ISM,
    // MF_PREFIX,
    // MF_SUFFIX,
    // MF_ATOMIC,
    // MF_RAW,
    // LB_SCTOR,
    // LB_LCTOR,
    // LB_CCTOR,
    // LB_MCTOR,
    // LB_DTOR,
    // LB_MEMBER,
    // LB_WHERE,
    // LB_MOVE,
    // LB_NEGATIVE,
    // LB_BITREV,
    // LB_INCREASE,
    // LB_DECREASE,
    // LB_INDEX,
    // LB_ADD,
    // LB_SUB,
    // LB_MUL,
    // LB_DIV,
    // LB_MOL,
    // LB_BITAND,
    // LB_BITOR,
    // LB_BITXOR,
    // LB_SHL,
    // LB_SHR,
    // LB_LT,
    // LB_GT,
    // LB_LE,
    // LB_GE,
    // LB_EQ,
    // LB_NE,
    // LB_ASSIGN,
    // PP_THEN,
    // PP_ON,
};

/**
 * @enum-class PVT : 约定
 * @desc :
 *  Alioth 0.3.1编译器将约定判定从CT中剥离出来
 */
enum class PVT {
    PRIVATE, // private -
    PROTECTED, // protected *
    PUBLIC, // public +
    ABSTRACT, // abstract
};

/**
 * @class token : 记号
 * @desc :
 *  记号可以用于表示一个词法符号或一个语法符号
 *  记号携带了此记号的文本内容和起止位置
 *  对于一个非终结符,文本内容无效
 */
class token {

    public:
        /**
         * @member id : 符号ID
         * @desc :
         *  用于区分不同词法记号的符号,在各个阶段用于标记单词类型
         */
        int         id;

        /**
         * @member bl : 起始行
         * @desc :
         *  用于标定此记号在源代码中的起始行
         */
        int         bl;

        /**
         * @member bc : 起始列
         * @desc :
         *  用于标定此记号在源代码的起始列
         */
        int         bc;

        /**
         * @member el : 终止行
         * @desc :
         *  用于标定此记号在源代码中的终止行
         */
        int         el;

        /**
         * @member ec : 终止列
         * @desc :
         *  用于标定此记号在源代码中的终止列
         */
        int         ec;

        /**
         * @member tx : 文本
         * @desc :
         *  用于记录记号实际上的书写形式
         *  对非终结符,此内容不一定有效
         */
        std::string tx;

    public:
        /**
         * @constructor : 构造方法
         * @desc :
         *  @form<1> : 构造一个空的记号，存入VT::R::ERR
         *  @form<2> : 构造一个空的记号,指定其终结符或非终结符
         *  @form<3> : 构造一个"LABEL"终结符,同时指定其文本
         *  @form<4> : 提供默认的拷贝构造方法
         *  @form<5> : 提供默认的移动构造方法
         */
        token();
        token( int id );
        token( const std::string& content );
        token( const token& ) = default;
        token( token&& ) = default;

        /**
         * @operator = : 赋值运算
         * @desc :
         *  token作为信息载体,可以被赋值而完全修改
         */
        token& operator = ( const token& ) = default;
        token& operator = ( token&& ) = default;

        /** 
         * @operator == : 判断两个记号是否相等
         * @desc :
         *  相等的条件是
         *      1： 确定书写格式的记号，只比对记号id
         *      2:  非终结符只比对记号id
         *      3:  不确定书写格式的记号，比对文本内容
         *      4:  VT::R::ERR与任何记号都不等价，包括它自己
         * @param another: 另一个词法符号
         * @return bool : 两个记号是否等价
         */
        bool operator == ( const token& another ) const;
        bool operator != ( const token& another ) const;

        /**
         * @destructor : 析构方法
         * @desc :
         *  提供默认析构方法
         */
        ~token() = default;

        /**
         * @operator std::string : 作文本使用
         * @desc :
         *  此方法先查询词汇表,若终结符与其中某项对应
         *  直接输出对应文本,忽略tx成员.
         *  若终结符不在词汇表中记录,则直接返回tx成员.
         * @return std::string
         */
        operator std::string()const;

        /**
         * @operator bool : 作布尔使用
         * @desc :
         *  此方法检查记号是否确定为一个VT::R::ERR终结符
         *  若成立则返回false,否则返回true
         * @return bool : 记号是否有效
         */
        explicit operator bool()const;

        /**
         * @method is : 用于判断文法符号
         * @desc :
         *  @form<1> : 判断文法符号是否为终结符或非终结符t
         *      @param t : 终结符或非终结符
         *  @form<2> : 判断文法符号是否属于某一类文法符号
         *      @param c : 符号分类
         *  @form<3> : 判定文法符号是否满足某词法约定
         *      @param p : 约定
         *  @form<4> : 方便用于连续进行多个判断的模板
         *      @param args : 任意个数的符号,或分类
         *  @return bool : 有任何判断成立,即返回true,否则返回false
         */
        bool is( int t )const;
        bool is( CT c )const;
        bool is( PVT p )const;
        template<typename...Args>bool is(Args ...args)const{return (... or is(args));}
        bool islabel()const;

        /**
         * @method extractContent : 提取内容
         * @desc :
         *  用于对STRING和CHARACTER提取文本内容
         *  此方法不提取层嵌语法结构
         *  对于其他token,直接返回文本内容
         * @return tuple<bool,string,Diagnostics> : 提取是否成功，提取内容，诊断信息
         */
        tuple<bool,string,Diagnostics> extractContent()const;

    public:
        static token line(int l) {token t;t.bl = l;t.bc = 0;return t;}
        static bool isvn( int id ) { return id == VT::R::ERR or id == VT::R::END or id == VT::R::BEG or (id > VT::min and id < VT::max); }
        static bool isvt( int id ) { return id > VN::min and id < VN::max; }
        static bool islabel( const string& str );
};

using tokens = chainz<token>;

}

#endif