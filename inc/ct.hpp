#ifndef __ct__
#define __ct__

namespace alioth {

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
    OP_LABEL,           //运算符标签

    IMPLEMENTATION,     //表达式,分支,循环,流控制,块
    STATEMENT,

    TERMINATOR,         //) ] } , : ;
};

}
#endif