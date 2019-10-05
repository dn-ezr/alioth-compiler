#ifndef __air_gvi__
#define __ari_gvi__

#include "agent.hpp"
#include "chainz.hpp"

/** gvi : global variable initializer --- 全局变量初始化器 */

namespace alioth::air {

class ConstantValue;
using $ConstantValue = agent<ConstantValue>;
class GVI;
using $GVI = agent<GVI>;

/**
 * @class GVI : 全局变量初始化器
 * @desc :
 *  全局变量需要使用全局变量初始化器来进行初始化
 */
class GVI : public thing {

    virtual ~GVI();
};

/**
 * @class ZeroGVI : 全零初始化器
 * @desc :
 *  全0初始化器用于将全局变量所占用的空间全部初始化位0
 */
class ZeroGVI : public GVI {

    virtual ~ZeroGVI();
};

/**
 * @class ConstantGVI : 常量初始化器
 * @desc :
 *  常量初始化器使用常量初始化全局变量
 */
class ConstantGVI : public GVI {

    /**
     * @class value : 常量值
     * @desc : 用于构造常量全局变量的常量值 */
    $ConstantValue value;

    ~ConstantGVI();
};

/**
 * @class ArrayGVI : 数组初始化器
 * @desc :
 *  数组初始化器用于初始化数组常量
 */
class ArrayGVI : public GVI {

    /**
     * @member elements : 数组元素
     * @desc : 数组初始化器的每个元素 */
    chainz<$GVI> elements;

    ~ArrayGVI();
};

/**
 * @class StringGVI : 字符串初始化器
 * @desc :
 *  使用字符串初始化i8[]全局变量时的特殊初始化器
 */
class StringGVI : public GVI {

    /**
     * @member content : 字符串内容
     * @desc : 不包含双引号，并且直接包含转义字符的字符串内容 */
    string content;

    ~StringGVI();
};

/**
 * @class CompositeGVI : 复合类型初始化器
 * @desc :
 *  用于构造复合数据类型的全局变量初始化器
 */
class CompositeGVI : public GVI {

    /**
     * @member members : 成员
     * @desc :  全局变量的成员 */
    chainz<$GVI> members;

    ~CompositeGVI();
};

}

#endif