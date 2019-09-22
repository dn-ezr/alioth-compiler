#ifndef __compiler__
#define __compiler__

#include "jsonz.hpp"
#include "target.hpp"
#include "space.hpp"
#include "diagnostic.hpp"
#include "syntax.hpp"

namespace alioth {

/**
 * @class CompilerContext : 编译器上下文
 * @desc :
 *  编译器用于管理资源的上下文，包含了文档，模块，片段之间的关联关系
 */
class CompilerContext {
    
    private:

        /**
         * @member work : 模块部署信息
         * @desc : 工作空间中的模块部署信息 */
        map<string,$signature> work;

        /**
         * @member root : 模块部署信息
         * @desc : 根空间中的模块部署信息 */
        map<string,$signature> root;

        /**
         * @member package : 模块部署信息
         * @desc : <package-name,<module-name,signature>> 包中的模块部署信息 */
        map<string,map<string,$signature>> package;

        /**
         * @member spaceEngine : 空间引擎
         * @desc : 用于支持数据交互功能 */
        SpaceEngine& spaceEngine;

        /**
         * @member diagnostics : 诊断信息
         * @desc : 用于报告诊断信息 */
        Diagnostics& diagnostics;
    
    public:

        /**
         * @ctor : 构造函数
         * @desc :
         *  用于构造编译器上下文，同时提供空间引擎
         * @param _spaceEngine : 空间引擎
         * @param _diagnostics : 日志仓库
         */
        CompilerContext( SpaceEngine& _spaceEngine, Diagnostics& _diagnostics );

        /**
         * @method getModule : 获取模块
         * @desc :
         *  此方法用于获取模块
         * @param name : 模块名
         * @param space : 模块所属空间，方法会自动提取主空间信息，忽略其他信息。
         * @param autogen : 是否自动构造不存在的模块
         * @return $signature : 模块唯一签名
         */
        $signature getModule( const string& name, srcdesc space, bool autogen = false );

        /**
         * @method getModule : 获取模块
         * @desc :
         *  此方法用于获取文档所属的模块
         *  若文档不存在，则获取失败
         * @param doc : 源文档描述符
         * @return $signature : 模块签名
         */
        $signature getModule( srcdesc doc );

        /**
         * @method getModule : 获取模块
         * @desc :
         *  此方法用于根据fragment中记录的源文档索引模块签名
         * @param fg : 片段
         * @return $signature : 模块签名
         */
        $signature getModule( $fragment fg );

        /**
         * @method getModules : 获取所有模块
         * @desc :
         *  获取某空间内的所有模块
         * @param space : 空间描述符
         * @return signatures : 某空间内的所有模块
         */
        signatures getModules( srcdesc space );

        /**
         * @method countModules : 统计空间内的模块
         * @desc :
         *  统计某空间内的模块个数
         * @param space : 空间描述符
         * @return size_t : 空间内模块的个数
         */
        size_t countModles( srcdesc space );

        /**
         * @method deleteModule : 删除模块
         * @desc :
         *  删除一个模块，描述此模块的所有文档，所有属于此模块的语法树
         * @param sig : 模块签名
         * @return bool : 删除动作是否成功
         */
        bool deleteModule( $signature sig );

        /**
         * @method loadModules : 从空间加载所有模块
         * @desc :
         *  此方法用于从某个主空间加载所有的模块
         *  此方法首先清理空间，并试图从缓冲文件读取签名
         *  接下来方法验证所有读取的签名的正确性，并确保缓冲正确
         *  若缓冲文件与实际情况有所出入，则重新写入缓冲文件
         * @param space : 空间描述符，方法会自动提取主空间信息，忽略其他信息。
         * @return bool : 若加载成功，返回true,若加载失败，返回false,诊断信息会写入容器
         */
        bool loadModules( srcdesc space );

        /**
         * @method clearSpace : 清空空间
         * @desc :
         *  清除一个空间所包含的所有模块
         * @param space : 空间描述符，方法会自动提取主空间信息，忽略其他信息。
         * @return bool : 清除动作是否成功
         */
        bool clearSpace( srcdesc space );

        /**
         * @method loadDocument : 从空间加载文档
         * @desc :
         *  此方法用于从空间加载文档，若文档未曾变化，且未指定forceload动作会被取消，返回原有描述符
         *  若文档所关联的模块尚不存在会自动创建此模块。
         *  若文档不可达或不包含合法签名，加载失败，若存在原有文档，则删除。根据文档后缀名和是否存在原有文档决定是否给出报错。
         *  为节省开销，此时不建立语法树
         * @param doc : 文档描述符
         * @param forceload : 是否强制加载行为，即使从描述符看不出文档的变化。
         */
        fulldesc loadDocument( srcdesc doc, bool forceload = false );
        fulldesc loadDocument( fulldesc doc, bool forceload = false );

        /**
         * @method unloadDocument : 卸载文档
         * @desc :
         *  卸载一个文档，若文档不存在，则卸载失败。
         *  卸载后，删除文档相关联的语法树，若卸载后文档所属模块没有更多文档，且未设置keepmodule,则自动删除模块
         * @param doc : 欲删除的文档
         * @param keepmodule : 是否保留空的模块
         * @return bool : 是否成功卸载文档
         */
        bool unloadDocument( srcdesc doc, bool keepmodule = false );

        /**
         * @method registerFragment : 向文档绑定语法树
         * @desc :
         *  向文档绑定语法树，只要文档存在就会成功
         * @param doc : 文档描述符
         * @param fg : 片段
         * @return bool : 是否绑定成功
         */
        bool registerFragment( srcdesc doc, $fragment fg = nullptr );

        /** 
         * @method getFragment : 获取片段
         * @desc :
         *  此方法用于从源文档检索对应的语法树
         * @param doc : 文档描述符
         * @return $fragment : 片段
         */
        $fragment getFragment( srcdesc doc );
};

/**
 * @class AbstractCompiler : 抽象编译器
 * @desc :
 *  抽象编译器作为基类，统一所有编译器内核的共性。
 */
class AbstractCompiler {

    protected:

        /**
         * @enum DiagnosticMethod : 诊断方法
         * @desc :
         *  用于表述诊断方法的枚举
         */
        enum DiagnosticMethod {
            STRING, // 将诊断信息组织成为字符串
            JSON    // 将诊断信息组织成为JSON结构体
        };

        /**
         * @struct DiagnosticsDestnation : 诊断信息目的地
         * @desc :
         *  用于表述诊断信息的发送目标的结构体
         */
        struct DiagnosticDestination {

            /**
             * @member fd : 文件描述符
             * @desc : 若此文件描述符大于等于0，则将诊断信息写入此文件描述符所表示的文件
             *  若此文件描述符小于0，则将诊断信息写入uri指向的数据链路 */
            int fd;

            /**
             * @member uri : 统一资源标识
             * @desc : 当文件描述符无效时，此成员被启用 */
            Uri uri;
        };

    protected:

        /**
         * @member spaceEngine : 空间引擎
         * @desc : 空间引擎必须在基础编译器被完全初始化，并传递给其他编译器类 */
        SpaceEngine* spaceEngine = nullptr;

        /**
         * @member diagnosticEngine : 诊断引擎
         * @desc : 诊断引擎必须在基础编译器中被完全初始化，并传递给其他编译器类 */
        DiagnosticEngine* diagnosticEngine = nullptr;

        /**
         * @member diagnosticMethod : 诊断方法
         * @desc : 记录对诊断方法的配置 */
        DiagnosticMethod diagnosticMethod = STRING;

        /**
         * @member diagnosticDestination : 诊断流向
         * @desc : 记录诊断信息的流向配置 */
        DiagnosticDestination diagnosticDestination = {fd :1};

        /**
         * @member diagnostics : 诊断信息
         * @desc : 记录编译器在运行期间产生的诊断信息 */
        Diagnostics diagnostics;

    private:
        /**
         * @ctor : 构造函数
         * @desc :
         *  构造方法用于在初始情况下对配置进行基础设置，仅基础编译器有权限调用此函数
         */
        AbstractCompiler() = default;
        friend class BasicCompiler;

    protected:
        /**
         * @ctor : 移动构造
         * @desc :
         *  移动构造函数负责拷贝所有配置信息，此方法应当被所有编译器子类调用
         * @param compiler : 拷贝信息的来源
         */
        AbstractCompiler( AbstractCompiler&& compiler );

        /**
         * @dtor : 析构函数
         * @desc :
         *  析构函数负责主动析构内置的各个引擎
         */
        virtual ~AbstractCompiler();

        /**
         * @method execute : 执行
         * @desc :
         *  此方法用于触发编译器执行目标。
         */
        virtual int execute() = 0;

        /**
         * @method configureDiagnosticMethod : 配置诊断方法
         * @desc :
         *  通过命令行参数配置诊断方法，配置若产生了错误则自动填写诊断信息
         * @param m : 命令行参数中的诊断方法
         */
        bool configureDiagnosticMethod( const string& m );

        /**
         * @method gonfigureDiagnosticDestination : 配置诊断信息流向
         * @desc :
         *  通过命令行参数配置诊断信息的流向，配置若产生了错误则自动填写诊断信息
         * @param m : 命令行参数中的诊断信息流向
         */
        bool configureDiagnosticDestination( const string& d );
};

/**
 * @class BasicCompiler : 基础编译器
 * @desc :
 *  基础编译器用于在确定目标之前启动基础资源配置
 *  基础编译器也用于基于磁盘路径工作
 */
class BasicCompiler : public AbstractCompiler {
    
    private:

        /**
         * @member args : 命令行参数
         * @desc : 决定编译器整体行为的命令行参数序列 */
        chainz<string> args;

    public:
        BasicCompiler( int argc, char** argv );
        ~BasicCompiler();
        int execute() override;

        int version();
        int help();
        int init( const string& package );

};

/**
 * @class AliothCompiler : 语言编译器
 * @desc :
 *  用于执行需要编译流程的目标的编译器
 *  如Auto,Executable,Static,Dynamic,Validate
 */
class AliothCompiler : public AbstractCompiler {

    private:
        /**
         * @member target : 目标信息
         * @desc : 描述执行目标 */
        CompilingTarget target;

        /**
         * @member target_modules : 目标模块
         * @desc : 存储所有目标所涉及的模块的签名 */
        signatures target_modules;

        /**
         * @member context : 编译器上下文
         * @desc : 用于集中管理编译资源的上下文环境 */
        CompilerContext context;
    public:
        /**
         * @ctor : 构造函数
         * @desc :
         *  构造函数从基础编译器构造Alioth编译器，并接受编译型目标配置
         * @param basic : 基础编译器，用于初始化基础配置
         * @param compilingTarget : 编译型目标，携带编译任务的具体目标信息
         */
        AliothCompiler( AbstractCompiler& basic, CompilingTarget compilingTarget );
        ~AliothCompiler();
        int execute() override;
    
    private:

        /**
         * @method detectInvolvedModules : 检测涉及模块
         * @desc :
         *  将所有此编译任务所涉及的模块的签名装入target_modules
         * @return bool : 若执行成功返回true
         */
        bool detectInvolvedModules();

        /**
         * @method confirmModuleCompleteness : 检测模块完备性
         * @desc :
         *  检查模块是否存在，模块的依赖是否完整等信息
         * @param name : 工作空间中的某个模块的模块名
         * @return bool : 返回模块是否完备
         */
        bool confirmModuleCompleteness( const string& name );

        /**
         * @method confirmModuleCompleteness : 检测模块完备性
         * @desc :
         *  此方法主要用于检测给定模块的依赖是否正确，包括依赖是否重复，依赖是否可达，依赖是否循环等。
         *  检查结果正确的模块将被放入target_modoules容器中。
         * @param mod : 待检查的模块
         * @param padding : 挂起的依赖，用于检查是否存在循环依赖
         * @return bool : 返回模块是否完备
         */
        bool confirmModuleCompleteness( $signature mod, chainz<$signature> padding = {} );

        /**
         * @method calculateDependencySpace : 解算依赖空间
         * @desc :
         *  解算依赖描述符所指向的模块所在的空间
         *  此方法不依赖目的模块或目的空间真实存在
         *  此方法要求依赖描述符所在的模块签名的空间被正确填写
         *  此方法的计算结果不一定是一个可达的空间！
         * @param dep : 依赖描述符
         * @return srcdesc : 空间描述符
         */
        srcdesc calculateDependencySpace( $depdesc dep );

        /**
         * @method calcaulateDependencySignature : 解算依赖签名
         * @desc :
         *  此方法用于解算依赖描述符所指向的模块签名
         *  若依赖所指向的空间没有模块，则尝试加载空间中的所有模块
         *  若依然未找到模块，则失败
         * @param dep : 依赖描述符
         * @param space : [输出]若需要，用于存储依赖所在空间
         * @return $signature : 模块签名
         */
        $signature calculateDependencySignature( $depdesc dep, srcdesc* space = nullptr );
};

class PackageManager : public AbstractCompiler {

};

class RepositoryManager : public AbstractCompiler {

};

}

#endif