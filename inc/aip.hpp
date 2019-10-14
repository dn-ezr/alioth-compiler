#ifndef __aip__
#define __aip__

#include "agent.hpp"

namespace alioth::protocol {

#define CONVERTER(type, name, member) type name(){return (type)member;}

/**
 * @enum Action : 包所表达的动作，分为请求和响应 */
enum Action {REQUEST, RESPOND};

/**
 * @enum status : 响应状态 */
enum Status {
    SUCCESS,
    FAILED,
    TIMEOUT,
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
     * @title DIAGNOSTICS: 诊断
     * @desc :
     *  @request(ide): 请求语义检查和诊断信息
     *      @param targets --- [string]: 目标模块列表
     *  @respond(compiler): 返回语义检查结果
     *      @param status : 响应状态
     *      @param diagnostics --- [json formated diagnostics] : 诊断信息
     */
    DIAGNOSTICS,
    
    /**
     * @title CONTENTS: 枚举
     * @desc :
     *  @request(compiler): 请求枚举内容
     *      @param uri --- string: 路径URI
     *  @respond(ide): 返回枚举内容
     *      @param status : 响应状态
     *      @param data {filename:{size,mtime,dir}} : 内容描述
     */
    CONTENTS,

    /**
     * @title WORKSPACE: 设置工作空间
     * @desc :
     *  @request(ide): 请求设置编译器的工作空间
     *      @param uri --- string: 工作空间URI
     *  @respond(compiler): 返回成功
     *      @param status : 响应状态
     */
    WORKSPACE,
};

string TitleStr( Title title );

/* ===> Parameters <=== */

something(Parameter);
struct Parameter : public basic_thing {

    struct Request {
        something(Diagnostics);
        something(Workspace);
    };
    struct Respond {
        something(Content);
        something(Contents);
    };
};

struct Parameter::Request::Diagnostics : public Parameter {
    chainz<string> targets;
};
struct Parameter::Request::Workspace : public Parameter {
    string uri;
};
struct Parameter::Respond::Content : public Parameter {
    string data;
};
struct Parameter::Respond::Contents : public Parameter  {
    struct item {
        size_t size;
        time_t mtime;
        bool dir;
    };

    map<string,item> data;
};

/* ===> Extensions <=== */

something(Extension);
struct Extension : public basic_thing {

    something(Request);
    something(Respond);
};

struct Extension::Request : public Extension {
    /**
     * @member params : 扩展
     * @desc : 根据不同的请求继续扩展包的内容 */
    $Parameter params;

    CONVERTER(Parameter::Request::$Diagnostics, diagnostics, params)
    CONVERTER(Parameter::Request::$Workspace, workspace, params)
};

struct Extension::Respond : public Extension {

    /**
     * @member status : 状态
     * @desc : 响应的状态，一般影响响应的其他参数的存在性 */
    Status status;

    /**
     * @member params : 参数
     * @desc : 包含请求包参数的扩展 */
    $Parameter params;

    
    CONVERTER(Parameter::Respond::$Content, content, params)
    CONVERTER(Parameter::Respond::$Contents, contents, params)
};

/* ===> Package <=== */

something(Package);using Packages = chainz<$Package>;
struct Package : public basic_thing {

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
    enum Title title;

    /**
     * @member extension : 包扩展
     * @desc : 包扩展的内容根据包的action和title的不同选取不同的扩展 */
    $Extension extension;

    CONVERTER(Extension::$Request, request, extension)
    CONVERTER(Extension::$Respond, respond, extension)
};

}

#endif