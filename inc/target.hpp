#ifndef __target__
#define __target__

#include <string>
#include "chainz.hpp"

namespace alioth {
using namespace std;

/**
 * @class Target
 * @desc :
 *  目标是一个抽象概念，描述编译器运行目的。
 */
struct Target {

    /**
     * @enum Indicator ： 指示器
     * @desc :
     *  在最高层次描述任务目的
     */
    enum Indicator {
        AUTO,
        LLVMIR,
        EXECUTABLE, 
        STATIC, 
        DYNAMIC, 
        VALIDATE,
        PACKAGE,
        INSTALL,
        UPDATE,
        REMOVE,
        PUBLISH,
        INIT,
        HELP,
        VERSION, };

    /** 
     * @member indicator : 目标指示器 
     * @desc : 目标指示器用于判断目标的具体类型*/
    Indicator indicator;
};

/**
 * @struct CompilingTarget : 编译型目标
 * @desc:
 *  使用编译工作流的目标都被归类为编译型目标
 */
struct CompilingTarget : public Target {
    /** 
     * @member name : 目标名称
     * @desc : 目标产物的名称主干部分，对于动态和静态链接库，编译器对目标名称会有所加工 */
    string name;

    /**
     * @member modules : 模块名录
     * @desc : 要编译的模块的名录，若此名录为空，则编译所有扫描到的模块 */
    chainz<string> modules;

    /**
     * @member variables : 编译器变量
     * @desc : 编译器变量通过命令行或配置文件指定，用于在源码中提供编译期静态求值的变量，可以用于拼接依赖包名称 */
    json variables;
};

/**
 * @struct PackageTarget : 打包目标
 * @desc :
 *  用于打包的目标
 */
struct PackageTarget : public Target {

    /** 
     * @member name : 目标名称
     * @desc : 打包的包名，在packages.json中必须存在对应的配置 */
    string name;

    /** 
     * @member sections : 节名录
     * @desc : 将要打包的节，此名录不能为空 */
    chainz<string> sections;
};

/**
 * @struct InstallTarget : 安装目标
 * @desc :
 *  用于安装包的目标
 */
struct InstallTarget : public Target {

    /** 
     * @member package : 包
     * @desc : 包的文件路径名或能够获取包的URI */
    string package;

    /**
     * @member sections : 节名录
     * @desc : 描述要安装的节，此名录不可省 */
    chainz<string> sections;
};

/**
 * @struct UpdateTarget : 更新目标
 * @desc :
 *  用于更新一个包的目标
 */
struct UpdateTarget : public Target {

    /** 
     * @member package : 包名
     * @desc : 要更新的包的包名 */
    string package;

    /**
     * @member major : 主版本号
     * @desc : 主版本号，负数表示取最大值 */
    int major = -1;

    /**
     * @member minor : 副版本号
     * @desc : 副版本号，负数表示取最大值 */
    int minor = -1;

    /**
     * @member patch : 补丁号
     * @desc : 补丁号，负数表示取最大值
     */
    int patch = -1;
};

/**
 * @struct RemoveTarget : 删除目标
 * @desc :
 *  用于移除一个已经安装的包的目标
 */
struct RemoveTarget : public Target {

    /**
     * @member package : 包名
     * @desc : 要删除的包的包名*/
    string package;
    /**
     * @member sections : 节名录
     * @desc : 要删除的节，若此表为空，则删除所有的节*/
    chainz<string> sections;
};

/**
 * @struct PublishTarget : 发布目标
 * @desc :
 *  用于发布一个包的目标
 */
struct PublishTarget : public Target {

    /**
     * @member package : 包名
     * @desc : 包的名称或包文件的文件名 */
    string package;
    
    /**
     * @member sections : 节名录
     * @desc : 若package为包名，不能为空；若package是包文件名，此名录必须为空 */
    chainz<string> sections;

    /**
     * @member repository : 仓库
     * @desc : 目标仓库的URL */
    string repository;

    /**
     * @member mode : 模式
     * @desc : 发布包所使用的模式 */
    enum Mode {AUTO,ONLINE,OFFLINE} mode = AUTO;
};

/**
 * @struct InformationTarget : 信息目标
 * @desc : 有关打印信息，辅助开发的目标
 */
struct InformationTarget : public Target {

    /**
     * @member package : 包名
     * @desc : 仅在初始化时启用，指定包的名称 */
    string package;
};

}

#endif