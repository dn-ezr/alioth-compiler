#ifndef __air_value__
#define __air_value__

#include "agent.hpp"
#include "chainz.hpp"
#include "air_type.hpp"
#include "air_gvi.hpp"

namespace alioth::air {

class Value;
using $Value = agent<Value>;
using Values = chainz<$Value>;
class Block;
using $Block = agent<Block>;
using Blocks = chainz<Blocks>;
class Inst;
using $Inst = agent<Inst>;
using Insts = chainz<$Inst>;

/**
 * @class Value : 值
 * @desc :
 *  值是AIR中的一等公民，大多数概念都继承自值 
 */
class Value {

    /**
     * @member type : 值的数据类型
     * @desc : 值的数据类型，对全局变量来说，此属性表达全局变量的地址的类型，而不是变量的类型，这确保了一个GV可以被直接使用 */
    $Type type;

    virtual $Type getType()const;
};

/**
 * @class User : 用户
 * @desc :
 *  用户表示对其他值的使用，用于表现变量之间的依赖关系
 *  为寄存器选择提供依据
 */
class User : public Value {

    /**
     * @member use : 变量代理
     * @desc : 使用的变量的代理 */
    Values use;
};

/**
 * @class GlobalValue : 全局值
 * @desc :
 *  全局值的名称需要进入符号表
 */
class GlobalValue : public User {

    string name;
};

/**
 * @class GlobalVariable : 全局变量
 * @desc :
 *  全局变量被用于定义或声明指向全局对象的指针
 */
class GlobalVariable : public GlobalValue {

    /**
     * @member varty : 变量数据类型
     * @desc : 区别于全局变量自身作为一个值的类型，全局变量所管理的变量的数据类型比全局变量的类型少一个指针层次 */
    $Type varty;

    /**
     * @member arr : 数组长度
     * @desc : 此数据用于确定全局变量是否成为数组，以及确定数组长度 */
    size_t arr;

    /**
     * @member initializer : 初始化器
     * @desc : 若指定初始化器，则全局变量被初始化，否则全局变量只分配空间 */
    $GVI initializer;
};

/**
 * @class Block : 块
 * @desc :
 *  AIR只能从一个块开始执行，从一个块结束执行
 *  跳转的目标必须是一个块的开始，跳转指令必须是一个块的最后一条指令
 *  块是指令序列，块作为Value代表着块的入口地址
 */
class Block : public Value, public Insts {

    /**
     * @member name : 块名称
     * @desc :
     *  块名称是局部名称，和函数中所有的局部名称竞争命名空间 */
    string name;
};

/**
 * @class Function : 函数
 * @desc :
 *  函数名代表着函数的入口地址指针，是一个函数指针
 *  函数的主体由块排成序列构成，函数从第一个块开始执行
 *  在某个块的最后一条指令结束执行
 */
class Function : public GlobalValue, public Blocks {

    /**
     * @member funty : 函数类型
     * @desc : Function作为一个GlobalValue的类型是函数指针，起本身的函数类型单独存储以便区分 */
    $Type funty;
};

}

#endif