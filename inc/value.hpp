#ifndef __value__
#define __value__

#include <llvm/IR/Value.h>
#include "agent.hpp"

namespace alioth {


struct eprototype;
using $eprototype = agent<eprototype>;

/** @enum addr_t : 描述运算值的地址类型 */
enum addr_t { none, direct, indirect };

/** 
 * @struct value_t : 运算值
 * @desc :
 *  运算值的作用是表述一个运算结果
 */
struct value_t {
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
         **/
        llvm::Value* value;

        /**
         * @member proto : 值对应的元素原型
         * @desc : 值的元素原型，其中数据类型必须保证总是有效的 */
        $eprototype proto;
};
}
#endif