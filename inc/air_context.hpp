#ifndef __air_context__
#define __air_context__

#include "air_type.hpp"

namespace alioth::air {

/**
 * @class Context : AIR上下文
 * @desc :
 *  AIR上下文用于在全局管理可能被重用的类型信息。
 *  AIR上下文用于在所有相关联的模块之间管理符号表。
 */
class Context {

    private:
        Types types;
};

}

#endif