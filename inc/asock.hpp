#ifndef __asock__
#define __asock__

#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "jsonz.hpp"
#include "chainz.hpp"
#include "aip.hpp"

/**
 * @file asock.hpp
 * @desc :
 *  asock是一组实现了Alioth Interacting Protocol的Api集合。
 */
namespace alioth {

struct Uri;
using namespace std;
using uistream = unique_ptr<istream>;
using uostream = unique_ptr<ostream>;
something(Socket);

/**
 * @class Socket : 交互器
 * @desc :
 *  交互器用于封装Alioth交互细节
 *  包括对2个缓冲的管理，消息的发送和接收等
 */
class Socket : public basic_thing {

    private:
        /**
         * @struct Transaction : 事务
         * @desc :
         *  用于表示一个请求对一个seq对应的响应的等待
         *  此结构的生命周期由InputStream创建
         *  由GuardInputStream终结，其他过程不得干预
         */
        struct Transaction : basic_thing {
            /**
             * @member cv : 条件变量
             * @desc : 等待线程使用此条件变量等待，Guard使用此条件变量通知 */
            condition_variable cv;
        }; using $Transaction = agent<Transaction>;

        /**
         * @struct InputStream : 输入流
         * @desc :
         *  用于抽闲每一个流的结构体
         */
        struct InputStream : public basic_thing, public std::mutex {

            /**
             * @member is : 输入流
             * @desc: 若输入流被关闭，is会被置空，此时接收请求的过程会自动模拟EXIT包 */
            uistream is;

            /**
             * @member requests : 接收到的请求包缓冲 */
            protocol::Packages requests;

            /**
             * @member cvreq : 用于nodify请求的条件变量 */
            condition_variable cvreq;

            /**
             * @member responds : 接收到的响应包缓冲 */
            map<long,protocol::$Package> responds;

            /**
             * @member transactions : 事务等待
             * @desc : 每个需要得到响应的请求都挂起一个事务逻辑 */
            map<int,$Transaction> transactions;

            /**
             * @member guard : 哨兵线程
             * @desc : 哨兵线程持续等待输入内容，并将输入内容整理到缓冲 */
            std::thread* guard;

            InputStream(uistream&& i):is(std::move(i)){}
            ~InputStream(){ if(guard) delete guard; }
        };using $InputStream = agent<InputStream>;

        /**
         * @struct OutputStream : 输出流
         * @desc :
         *  用于抽象每个输出流，并使用互斥锁同步输出操作
         */
        struct OutputStream : public basic_thing, public std::mutex {

            /**
             * @member os : 输出流 */
            uostream os;

            OutputStream( uostream&& o):os(std::move(o)){}
        };using $OutputStream = agent<OutputStream>;

    private:

        /**
         * @static-member istreams, ostreams : 全局流
         * @desc : 使用互斥锁对所有的流进行同步 */
        static std::map<Uri,$InputStream> istreams;
        static std::map<Uri,$OutputStream> ostreams;

        /**
         * @static-member repo_lock : 仓库锁
         * @desc : 对流仓库进行操作时，用于同步的互斥锁 */
        static std::mutex repo_lock;

        /**
         * @member seq_lock : 序列号锁
         * @desc : 用于确保序列号的操作全局同步的锁 */
        static std::mutex seq_lock;

        /**
         * @member global_seq : 全局序列号
         * @desc : 用于产生请求序列号的全局序列号 */
        static long global_seq;

    private:

        /**
         * @member in : 输入流 */
        $InputStream in;

        /**
         * @member out : 输出流 */
        $OutputStream out;

    public:

        Socket();
        Socket( const Socket& ) = delete;
        Socket( Socket&& );

        bool ActivateInputStream( const Uri& uri );
        bool ActivateOutputStream( const Uri& uri );

        long requestContent( const Uri& uri );
        long requestContents( const Uri& uri );

        void respondDiagnostics( long seq, const json& diagnostics );
        void respondSuccess( long seq, protocol::Title title );
        void respondFailure( long seq, protocol::Title title );

        void reportException( const string& msg );

        protocol::$Package receiveRespond( long seq );
        protocol::$Package receiveRequest();
    private:

        static $InputStream GetInputStream( const Uri& uri );
        static $OutputStream GetOutputStream( const Uri& uri );

        static void GuardInputStream( $InputStream s );

        static protocol::$Package ExtractPackage( const json& data );
        static protocol::$Package ExtractRequestPackage( const json& data, protocol::$Package package );
        static protocol::$Package ExtractRespondPackage( const json& data, protocol::$Package package );
        long sendRequest( protocol::Title title, json& pack );
        void sendRespond( long seq, protocol::Title title, json& pack );
        void sendPackage( json& pack );
};

}
#endif