#ifndef __compiler__
#define __compiler__

#include "jsonz.hpp"
#include "target.hpp"
#include "space.hpp"
#include "diagnostic.hpp"
#include "syntax.hpp"
#include "context.hpp"

namespace alioth {

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

        /**
         * @member full_interactive : 是否开启全交互模式
         * @desc : 全交互模式会使得Alioth编译器进入挂机状态，根据指令行动 */
        bool full_interactive = false;

        /**
         * @member msock : 交互套接字
         * @desc : 用于交互模式的交互套接字 */
        Socket msock;

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
         * @method execute_full_interactive : 执行全交互模式
         * @desc :
         *  进入全交互模式，等待请求，完成指定任务
         * @return int :　结束码
         */
        int execute_full_interactive();

        /**
         * @method enableFullInteractiveMode: 启动全交互模式
         * @desc :
         *  全交互模式将空间引擎和Alioth编译器都与指定输入输出流链接
         * @param input : 指定输入流
         * @param output : 指定输出流
         * @return bool : 交互模式是否启动成功
         */
        bool enableFullInteractiveMode( int input, int output );

        /**
         * @method detectInvolvedModules : 检测涉及模块
         * @desc :
         *  将所有此编译任务所涉及的模块的签名装入target_modules
         * @return bool : 若执行成功返回true
         */
        bool detectInvolvedModules();

        /**
         * @method performSyntaticAnalysis : 执行语法分析
         * @desc :
         *  为已经提取的目标模块执行语法分析，挂载所有的片段
         * @return bool : 是否成功
         */
        bool performSyntaticAnalysis();

        /**
         * @method performSyntaticAnalysis : 执行语法分析
         * @desc :
         *  为指定的模块执行语法分析，挂载片段
         * @param sig : 模块签名
         * @return bool : 是否成功
         */
        bool performSyntaticAnalysis( $signature sig );

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
         * @param _from : 此模块来自哪个依赖
         * @return bool : 返回模块是否完备
         */
        bool confirmModuleCompleteness( $signature mod, chainz<tuple<$signature,$depdesc>> padding = {}, $depdesc _from = nullptr );

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