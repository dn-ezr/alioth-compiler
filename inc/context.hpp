#ifndef __context__
#define __context__

#include "space.hpp"
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
         * @desc : <package-id,<module-name,signature>> 包中的模块部署信息 */
        map<string,map<string,$signature>> package;

        /**
         * @member spaceEngine : 空间引擎
         * @desc : 用于支持数据交互功能 */
        SpaceEngine& spaceEngine;

        /**
         * @member diagnostics : 诊断信息
         * @desc : 用于报告诊断信息 */
        Diagnostics& diagnostics;

        /**
         * @member cache_objects : 缓冲对象
         * ＠desc : 对每个空间的缓冲对象 */
        map<srcdesc,json> cache_objects;
    
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
         * @method getSpaceEngine: 获取空间引擎
         * @desc :
         *  受Context管辖的语法结构有时会需要使用Context的部分功能
         */
        SpaceEngine& getSpaceEngine();

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
         * @method syncModules : 同步模块部署信息
         * @desc :
         *  此方法用于将缓冲的模块部署信息同步到与实际情况一致
         *  并将最新的缓冲信息写入空间缓冲文件
         * @param space : 空间描述符，方法会自动提取主空间信息，忽略其他信息。
         * ＠return bool : 操作是否成功
         */
        bool syncModules( srcdesc space );

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
         * @method registerFragmentFailure : 登记片段失败
         * @desc :
         *  若构造语法树失败，应当记录失败时产生的诊断信息
         * @param doc : 文档描述符
         * @param info : 诊断信息
         * @return bool : 是否绑定成功
         */
        bool registerFragmentFailure( srcdesc doc, const Diagnostics& info );

        /** 
         * @method getFragment : 获取片段
         * @desc :
         *  此方法用于从源文档检索对应的语法树
         * @param doc : 文档描述符
         * @return $fragment : 片段
         */
        tuple<int,$fragment,Diagnostics> getFragment( srcdesc doc );
};

}

#endif