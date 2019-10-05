#ifndef __air_module__
#define __air_module__

#include "agent.hpp"
#include <string>

namespace alioth::air {

/**
 * @class Module : 模块
 * @desc :
 *  用于集中管理一个模块所拥有的所有信息
 */
class Module : public thing {

    /**
     * @member name : 模块名
     * @desc : 模块名称必须满足Alioth变量命名规则 */
    std::string name;
};

}

#endif