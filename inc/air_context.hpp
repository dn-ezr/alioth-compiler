#ifndef __air_context__
#define __air_context__

#include "air_type.hpp"
#include "air_module.hpp"

namespace alioth {

/**
 * @class AirContext : AIR上下文
 * @desc :
 *  AIR上下文用于在全局管理可能被重用的类型信息。
 *  AIR上下文用于在所有相关联的模块之间管理符号表。
 */
class AirContext {

    private:
        air::Types types;

    public:
};

}

#endif