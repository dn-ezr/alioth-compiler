#ifndef __aip__
#define __aip__

#include "agent.hpp"

namespace alioth {

/**
 * @enum Action : 包所表达的动作，分为请求和响应 */
enum Action {REQUEST, RESPOND};

/**
 * @constexpr status : 响应状态 */
class Status {
    public:
        static constexpr int SUCCESS = 0;
        static constexpr int FAILED = 1;
        static constexpr int TIMEOUT = 2;
};

/**
 * @enum Title : 动作的标题，是主要信息，不同动作的同一个标题可能有不同含义 */
enum Title {

    /**
     * @title CONTENT: 文本内容
     * @desc :
     *  @request(compiler): 请求文本内容
     *      @param uri --- string: 文本URI
     *  @respond(ide): 返回文本内容
     *      @param status : 响应状态
     *      @param data: 文本内容
     */
    CONTENT,

    /**
     * @title VALIDATION: 检查
     * @desc :
     *  @request(ide): 请求语义检查
     *      @param targets --- [string]: 目标模块列表
     *  @respond(compiler): 返回语义检查结果
     *      @param status : 响应状态
     *      @param diagnostics --- {uri:[string]} : 诊断信息
     */
    VALIDATION,

    /**
     * @title ENUMERATE: 枚举
     * @desc :
     *  @request(compiler): 请求枚举内容
     *      @param uri --- string: 路径URI
     *  @respond(ide): 返回枚举内容
     *      @param status : 响应状态
     *      @param data {filename:{size,timestamp}} : 内容描述
     */
    ENUMERATE,
};

struct PackageExtension : public basic_thing {

}; using $PackageExtension = agent<PackageExtension>;

/**
 * @struct BasicPackage : 基础包
 * @desc :
 *  基础包结构表达了包的基础信息
 */
struct BasicPackage : public basic_thing {

    /**
     * @member action : 动作 */
    Action action;

    /**
     * @member seq : 序列号
     * @desc : 
     *  响应的序列号应当对应于请求的序列号
     *  每个请求端持有自己的seq即可，不必刻意避免序列号重复
     *  若响应的序列号为0，被视为自动报告 */
    long seq;

    /**
     * @member timestamp : 时间戳
     * @desc : 暂时看来毫无用处... */
    time_t timestamp;

    /**
     * @member title : 请求标题
     * @desc : 请求标题决定了请求包的其他参数的意义 */
    Title title;

    /**
     * @member extension : 包扩展
     * @desc : 包扩展的内容根据包的action和title的不同选取不同的扩展 */
    $PackageExtension extension;
};
using $BasicPackage = agent<BasicPackage>;
using BasicPackages = chainz<$BasicPackage>;

struct RequestPackageExtension : public PackageExtension {
    /**
     * @member params : 扩展
     * @desc : 根据不同的请求继续扩展包的内容 */
    $PackageExtension params;
};

struct RespondPackageExtension : public PackageExtension {

    /**
     * @member status : 状态
     * @desc : 响应的状态，一般影响响应的其他参数的存在性 */
    int status;

    /**
     * @member paramｓ : 参数
     * @desc : 包含请求包参数的扩展 */
    $PackageExtension params;
};


struct RequestContentPackageExtension : public PackageExtension {
    string uri;
};

struct RespondContentPackageExtension : public PackageExtension {
    string data;
};

}

#endif