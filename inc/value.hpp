#ifndef __value__
#define __value__

#include <llvm/IR/Value.h>
#include "agent.hpp"

namespace alioth {


struct eprototype;
using $eprototype = agent<eprototype>;
struct value_t;
using $value = agent<value_t>;
using values = chainz<$value>;

/**
 * @enum addr_t : 描述运算值的地址类型 
 */
enum addr_t { 
    /** 描述立即数，没有地址, value即值(特别的，结构体使用指针) */
    none,
    
    /** 描述直接寻址, value 是 值的地址 */
    direct,
    
    /** 描述二级寻址， value 是代理体指针 */
    indirect,
    
    /** 描述代理寻址, value 是可执行的成员运算 */
    proxy
};

/** 
 * @struct value_t : 运算值
 * @desc :
 *  运算值的作用是表述一个运算结果
 */
struct value_t : basic_thing {
    public:

        /**
         * @member addr : 地址类型
         * @desc : 地址类型决定了value存储的内容 */
        addr_t addr;

        /**
         * @member value : 后端值
         * @desc : 根据地址类型存储不同的内容
         *  none (simple) -> obj
         *  direct (simple) -> *obj
         *  none | direct (struct) -> *obj
         *  indirect (simple|struct) -> reference
         *  proxy (simple|struct) -> function
         **/
        llvm::Value* value;

        /**
         * @member setter : 设置器
         * @desc :
         *  给proxy类型的值专用的设置器，对应于有参数的成员运算符
         */
        llvm::Function* setter;

        /**
         * @member host : 宿主
         * @desc :
         *  成员运算或成员方法的宿主 */
        $value host;

        /**
         * @member proto : 值对应的元素原型
         * @desc : 值的元素原型，其中数据类型必须保证总是有效的 */
        $eprototype proto;
};
}
#endif