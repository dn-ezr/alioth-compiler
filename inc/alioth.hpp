#ifndef __alioth__
#define __alioth__

/**
 * @namespace alioth : alioth命名空间
 * @desc :
 *  开发alioth编程语言所需的所有部件都部署在alioth命名空间中
 *  alioth开发组件中的类,主要分为信息载体和引擎两类.
 *  每个引擎都是一个模块的主体,实现了众多核心功能.
 *  而每个信息载体,是各个模块间沟通的基本单位,为了方便承载和共享信息
 *  通常设计为开放成员的形式,并仅包含一些简单的功能用于简化开发
 */

#include "chainz.hpp"
#include <tuple>

namespace alioth {

#define __language_ver__  std::tuple<int,int>(0,9)
#define __language_ver_str__ string("0.9")
#define __compiler_ver__ std::tuple<int,int,int>(0,2,0)
#define __compiler_ver_str__ string("0.3.0")

#ifdef __ALIOTH_DEBUG__
#define internal_error diagnostics[__FILE__]("internal",token::line(__LINE__))
#else
#define internal_error
#endif
}

#endif