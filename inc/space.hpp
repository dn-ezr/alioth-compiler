#ifndef __space__
#define __space__

#include <map>
#include <string>
#include <atomic>
#include <istream>
#include <ostream>
#include "chainz.hpp"
#include "jsonz.hpp"
#include "asock.hpp"
#include <memory>

namespace alioth {
class fdistream;
class fdostream;
using namespace std;
using uistream = unique_ptr<istream>;
using uostream = unique_ptr<ostream>;

constexpr int DOCUMENT = 0x80'00; //文档标志
constexpr int ROOT = 0x01'00; //根空间标志
constexpr int WORK = 0x02'00; //工作空间标志
constexpr int APKG = 0x03'00; //应用空间标志
constexpr int ARC = 0x00'01;
constexpr int BIN = 0x00'02;
constexpr int DOC = 0x00'03;
constexpr int INC = 0x00'04;
constexpr int LIB = 0x00'05;
constexpr int OBJ = 0x00'06;
constexpr int SRC = 0x00'07;
constexpr int EXT = 0x00'80; //扩展空间标志

constexpr int MAIN = 0x7f00;
constexpr int SUB = 0x00ff;

/**
 * @struct Uri : uri结构
 * @desc :
 *  根据RFC3986 --- STD66设计的URI结构
 *  scheme://user:password@host:port/path?query#fragment
 */
struct Uri {
    string scheme;
    string user;
    string password;
    string host;
    int port = -1;
    string path;
    string query;
    string fragment;

    static Uri FromString( const string& str );
    static Uri FromPath( const string& str );
    static bool IsPath( const string& str );
    static bool IsRelativePath( const string& str );
    static bool IsAbsolutePath( const string& str );
    operator string() const;
    explicit operator bool()const;

    bool operator == ( const Uri& ) const;
    bool operator <( const Uri& ) const;

    static const Uri Bad;
};

/**
 * @struct PckageLocator : 包定位器
 * @desc :
 *  包定位器的内容和包ID的内容基本一致，但其中只有publisher和name总是必须的字段
 *  其他字段可以根据编译器的当前配置进行自动填充
 *  格式: publisher.packageName-arch-platform:major.minor.patch sections
 *  样例: cn.org.alioth.stdlib:^
 *  样例: cn.org.alioth.stdlib:3.^ dev
 */
struct PackageLocator {

    public:
        static constexpr int MAIN = 1;
        static constexpr int DEV = 2;
        static constexpr int DOC = 3;
        static const string THIS_ARCH;
        static const string THIS_PLATFORM;

    public:
        /**
         * @member publisher : 发布者
         * @desc : 发布者身份是在alioth.org.cn上注册的命名空间 */
        string publisher;

        /**
         * @member name : 包名称
         * @desc : 包名称是最短的包的名称 */
        string name;

        /**
         * @member arch : 架构
         * @desc : [可选]包的目标机器的架构名称 */
        string arch;

        /**
         * @member platform : 平台
         * @desc : [可选]]包所在的操作系统名称或包的运行环境名称 */
        string platform;

        /**
         * @member major, minor, patch : 版本号
         * @desc :
         *  [可选]指定包的版本号，负数表示匹配能找到的最大版本号
         *  major : 主版本号，表示向下不兼容的重大升级
         *  minor : 副版本号，表示向下兼容的功能提升
         *  patch : 补丁号，表示对问题的修复或漏洞的修补 */
        int major = 0, minor = 0, patch = 0;

        /**
         * @member sections : 段
         * @desc : [可选]使用数字开关指定的段 */
        int sections = 0;

    public:

        /**
         * @operator bool : 判定定位器是否有效
         * @desc : 定位器的publisher或name为空则无效 */
        operator bool()const;

        /**
         * @method Parse : 解析一个包定位器
         * @desc : 从字符串解析一个包定位
         * @param id : 包含包定位器的字符串
         */
        static PackageLocator Parse( const string& id );

        /**
         * @param uriPath : 是否转换成uri路径字符串,若转换成路径，则不包含版本号信息，因为版本号与路径的映射关系由空间引擎管理 */
        string toString( bool uriPath = false, const string& suggest_arch = THIS_ARCH, const string& suggest_platform = THIS_PLATFORM )const;
};

/**
 * @struct srcdesc : 数据源描述符
 * @desc :
 *  数据源描述符能唯一确定一个数据源
 */
struct srcdesc {

    /**
     * @member flags : 标志位
     * @desc : 标志用于抽象描述数据源的空间定位 */
    int flags = 0;

    /**
     * @member name : 数据源名称
     * @desc : 数据源名称是短名称，不包含路径 */
    string name;

    /**
     * @member package : 包名
     * @desc : 当描述符描述的内容在安装的包中时，包名起作用 */
    string package;

    bool isSpace() const;
    bool isDocument() const;
    bool isMainSpace() const;
    bool isSubSpace() const;

    json toJson() const;
    static srcdesc fromJson( const json& object );

    bool operator < ( const srcdesc& desc ) const {
        return flags < desc.flags or name < desc.name or package < desc.package;
    }

    bool operator == ( const srcdesc& desc ) const {
        if( flags != desc.flags ) return false;
        if( isDocument() )
            if( name != desc.name ) return false;
        if( (flags & APKG) != 0 )
            if( package != desc.package ) return false;
        return true;
    }

    bool operator != ( const srcdesc& desc ) const {
        return !((*this) == desc);
    }

    explicit operator bool()const;

    const static srcdesc error;
};

/**
 * @struct fulldesc : 数据源全描述符
 * @desc :
 *  数据源全描述符在数据源描述符的基础上增添了
 *  时间戳和大小信息用于确定数据源的变化
 */
struct fulldesc : public srcdesc {

    /**
     * @member mtime : 修改时间戳
     * @desc : 修改时间戳是辅助信息，辅助判断数据源是否更新 */
    time_t mtime;

    /**
     * @member size : 数据源尺寸
     * @desc : 若可能的情况下，获取数据源尺寸 */
    size_t size;

    fulldesc() = default;
    fulldesc( const srcdesc& desc ):srcdesc(desc),mtime(0),size(0){}
    fulldesc( srcdesc&& desc ):srcdesc(std::move(desc)),mtime(0),size(0){}
    json toJson() const;
    static fulldesc fromJson( const json& object );
};

class SpaceEngine {

    public:

        static const char dirdvc;               //路径分隔符，字符
        static const string dirdvs;             //路径分隔符，字符串
        static const string default_work_path;  //默认工作路径
        static const string default_root_path;  //默认根路径

    private:
        /**
         * @struct DataSource : 数据源
         * @desc :
         *  数据源是空间引擎最基础的管理对象
         *  数据源是抽象对象，它是空间和文档的最高抽象层次
         */
        struct DataSource {

            /**
             * @member desc : 数据源描述符
             * @desc : 数据源的描述符用于确定数据源的唯一身份 */
            fulldesc desc;

            /**
             * @member mapping : 数据源映射
             * @desc : 将数据源映射为某个URI或PATH,若用于存储PATH则必须以'.'或'./'开头 */
            Uri mapping;
        };

        /**
         * @class Space : 空间
         * @desc :
         *  空间是对空间引擎中空间的抽象实现
         *  用于存储空间的相关配置
         */
        struct Space : public DataSource {
        };


        struct Document : public DataSource {
        };

    private:

        /**
         * @member mwork : 工作空间配置
         * @desc : 用于存储工作空间的相关配置 */
        Space mwork;

        /**
         * @member mroot : 根空间配置
         * @desc : 用于存储根空间相关的配置信息 */
        Space mroot;

        /**
         * @member mapkg : 应用空间配置
         * @desc : 用于存储各个应用空间的相关配置 */
        map<string,Space> mapkg;

        /**
         * @member interactive : 交互模式
         * @desc : 描述交互模式是否被启动 */
        bool interactive;

        /**
         * @member msock : 交互套接字
         * @desc : 用于交互模式的交互套接字 */
        $Socket msock;

        /**
         * @member arch : 架构 */
        string arch;

        /**
         * @member platform : 平台 */
        string platform;

    public:

        /**
         * @ctor : 构造器
         * @desc :
         *  构造器初始化所有的参数为默认参数
         */
        SpaceEngine();

        /**
         * @method setMainSpaceMapping : 设置主空间映射
         * @desc :
         *  此方法可以用于设置主空间的映射,当编译器需要处理远程包时可能需要用到。
         * @param space : 主空间标记
         * @param mapping : 重映射URI或路径
         * @param package : 当主空间标记为APKG时，此参数启用
         * @return bool : 映射是否成功
         */
        bool setMainSpaceMapping( int space, const string& mapping, const string& package = "" );

        /**
         * @method setArch : 设置架构
         * @param ar : 架构信息
         */
        void setArch( const string& ar );

        /**
         * @method getArch : 获取架构
         */
        string getArch()const;

        /**
         * @method setPlatform : 设置平台
         * @param pm : 平台信息
         */
        void setPlatform( const string& pm );

        /**
         * @method getPlatform : 获取平台
         */
        string getPlatform()const;

        /**
         * @method enumerateContents : 枚举内容物
         * @desc :
         *  枚举空间内的所有内容，将完整的描述符返回
         * @param desc : 要枚举的空间的描述符，若此描述符不指向一个空间则抛出异常。
         * @return chainz<fulldesc> : 返回所有可枚举项的完全描述符
         */
        chainz<fulldesc> enumerateContents( const srcdesc& desc );

        /**
         * @method enumeratePublisher : 枚举发布者
         * @desc :
         *  枚举所有已知的发布者
         * @return chainz<string> : 所有发布者名称
         */
        chainz<string> enumeratePublishers();

        /**
         * @method enumeratePackages : 枚举包
         * @desc :
         *  Alioth约定必须将包安装在root/pkg/中
         *  此方法用于简化枚举包的过程
         * @param publisher : 包所属的发布者
         * @return chainz<string> : 所有的包名
         */
        chainz<string> enumeratePackages( const string& publisher );

        /**
         * @method enumerateVersions : 枚举版本号
         * @desc :
         *  枚举一个包已经安装的所有版本号
         * @param publisher : 发布者
         * @param package : 包名
         * @return chainz<string> : 所有的版本号
         */
        chainz<string> enumerateVersions( const string& publisher, const string& package );

        /**
         * @method createDocument : 创建文档
         * @desc :
         *  创建文档，此方法尝试确保文档所在的空间被创建
         * @param desc : 要创建的文档的描述符
         * @param mod : 文档的权限配置，默认0644
         * @return bool : 文档创建是否成功
         */
        bool createDocument( const srcdesc& desc, int mod = 0644 );

        /**
         * @method createSubSpace : 创建子空间
         * @desc :
         *  此方法用于创建一个子空间，并设置其权限为0755
         * @param desc : 子空间的描述符
         * @return bool : 子空间是否创建成功
         */
        bool createSubSpace( const srcdesc& desc );

        /**
         * @method openDocumentForRead : 打开文档以读取
         * @desc :
         *  此方法用于打开一个文档的读取输入流
         * @param desc : 文档的描述符
         * @return uistream : 输入流指针
         */
        uistream openDocumentForRead( const srcdesc& desc );

        /**
         * @method openDocumentForWrite : 打开文档以写入
         * @desc :
         *  此方法用于打开一个文档的写入输出流
         * @param desc : 文档的描述符
         * @return uostream : 输出流指针
         */
        uostream openDocumentForWrite( const srcdesc& desc );

        /**
         * @method reachDataSource : 检查数据源可达性
         * @desc :
         *  检查数据源是否可达
         * @param desc : 数据源描述符
         * @return bool : 数据源是否可达
         */
        bool reachDataSource( const srcdesc& desc );

        /**
         * @method statDataSource : 检查数据源详情
         * @desc :
         *  检查数据源是详情
         * @param desc : 数据源描述符
         * @return fulldesc : 数据源全描述符
         */
        fulldesc statDataSource( const srcdesc& desc );

        /**
         * @method statDataSource : 检查数据源详情
         * @desc :
         *  检查数据源详情，若数据源不在管控范围内，则无论是否可达，都返回error
         * @param uri : 数据源的统一资源标识符
         * @return fulldesc : 数据源全描述符
         */
        fulldesc statDataSource( Uri uri );

        /**
         * @method getPath : 获取文件路径
         * @desc :
         *  此方法用于获取一个描述符所描述的资源的文件路径。
         *  若资源并非部署在本地，此方法抛出异常。
         * @param desc : 资源的描述符
         * @return string : 字符串格式的文件路径
         */
        string getPath( const srcdesc& desc );

        /**
         * @method getUri : 获取资源URI
         * @desc :
         *  此方法用于获取一个描述符所描述的资源的URI
         * @param desc : 资源的描述符
         * @return string : 字符串格式的资源URI
         */
        Uri getUri( const srcdesc& desc );

        /**
         * @method enableInteractiveMode : 启动交互模式
         * @desc :
         *  此方法用于启动交互模式，并设置交互输入输出文件描述符
         * @param input : 输入流文件描述符
         * @param output : 输出流文件描述符
         */
        bool enableInteractiveMode( int input, int output );

        /**
         * @method OpenStramForRead : 打开流以读取数据
         * @desc :
         *  此方法提供与配置无关的打开流用于读取数据的接口
         * @param fd : 文件描述符
         * @return uistream : 唯一流指针
         */
        static uistream OpenStreamForRead( int fd );

        /**
         * @method OpenStramForRead : 打开流以读取数据
         * @desc :
         *  此方法提供与配置无关的打开流用于读取数据的接口
         * @param fd : 流所指向的资源的URI
         * @return uistream : 唯一流指针
         */
        static uistream OpenStreamForRead( Uri uri );

        /**
         * @method OpenStramForRead : 打开流以写入数据
         * @desc :
         *  此方法提供与配置无关的打开流用于写入数据的接口
         * @param fd : 文件描述符
         * @return uistream : 唯一流指针
         */
        static uostream OpenStreamForWrite( int fd );

        /**
         * @method OpenStramForRead : 打开流以写入数据
         * @desc :
         *  此方法提供与配置无关的打开流用于写入数据的接口
         * @param fd : 流所指向的资源的URI
         * @return uistream : 唯一流指针
         */
        static uostream OpenStreamForWrite( Uri uri );

        /**
         * @method ReachDataSource : 检查数据源是否可达
         * @desc :
         *  检查数据源是否可达
         * @param uri : 数据源的统一资源标识符
         * @return bool : 数据源是否可达
         */
        static bool ReachDataSource( Uri uri );

        static bool IsSpace( int );
        static bool IsDocument( int );
        static bool IsMainSpace( int );
        static bool IsSubSpace( int );
        static int PeekMain( int );
        static int PeekSub( int );
        static int HideMain( int );
        static int HideSub( int );
};

/**
 * @class fdistream : 文件描述符输入流
 * @desc :
 *  用于从文件描述符建立输入流，并管理缓冲内存分配
 */
class fdistream : public istream {
    public:
        fdistream( int fd );
        virtual ~fdistream();
};

/**
 * @class fdostream : 文件描述符输入流
 * @desc :
 *  用于从文件描述符建立输入流，并管理缓冲内存分配
 */
class fdostream : public ostream {
    public:
        fdostream( int fd );
        virtual ~fdostream();
};

}

#endif