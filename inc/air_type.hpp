#ifndef __air_type__
#define __air_type__

#include <string>
#include "agent.hpp"
#include "chainz.hpp"
#include "air_id.hpp"

namespace alioth::air {

class Type;
using $Type = agent<Type>;
using Types = chainz<$Type>;

/**
 * @class Type : 类型
 * @desc :
 *  描述一个类型，可以是基础数据类型，指针数据类型，复合数据类型，函数类型
 */
class Type : public thing {

    /**
     * @member id : 类型ID
     * @desc :用于进行RTTI的类型ID */
    id_t id;

    virtual ~Type();
};

/**
 * @class PointerType : 指针数据类型
 * @desc :
 *  只有携带PointableTypeMask的类型才能被指针指向
 */
class PointerType : public Type {

    /**
     * @member org : 子类型
     * @desc : 指针的目标数据类型 */
    Type sub;
};

/**
 * @class CompositeType : 复合数据类型
 * @desc :
 *  复合数据类型
 */
class CompositeType : public Type {

    /**
     * @member name : 类型名
     * @desc : 复合数据类型名称去掉'@'前缀后的内容 */
    std::string name;

    /**
     * @member sub : 子类型
     * @desc : 复合数据类型的子类型 */
    Types sub;
};

/**
 * @class FunctionType : 函数类型
 * @desc :
 *  函数类型用于描述函数的返回值类型和参数类型,其中返回值类型只能是基础数据类型或指针类型
 *  [[!!!]由于AIR仍然不能与寄存器相关，所以函数**本应该**依然可以返回结构体]
 *  [注：这样的设计理应是合理的，因为很可能返回结构体的行为在机器层还能得到进一步优化(存疑)
 *      但是Alioth已经处理了函数的返回值，任何函数都会返回基础数据类型
 *      由于Alioth要协调返回的结构体的指针和this引用的指针的顺序，所以此处不能擅自优化]
 */
class FunctionType : public Type {

    /**
     * @member rtty : 返回值数据类型 
     * @desc : 仅基础数据类型，空数据类型，指针数据类型有效 */
    Type rtty;

    /**
     * @member argty : 参数数据类型
     * @desc : 参数数据类型,除了函数类型以外都有效，函数类型指针也是有效的 */
    Types argty;
};

}

#endif