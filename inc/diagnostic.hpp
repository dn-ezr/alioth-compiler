#ifndef __diagnostic__
#define __diagnostic__

#include <string>
#include "token.hpp"
#include "chainz.hpp"
#include "jsonz.hpp"
#include <tuple>
#include <map>

namespace alioth {
using namespace std;

struct Diagnostic;

/**
 * @struct Diagnostics : 诊断信息组
 * @desc :
 *  诊断信息组用于成组管理诊断信息，同时它也是一个诊断信息生成器
 */
class Diagnostics : public chainz<Diagnostic> {

    private:
        /**
         * @member prefix : 前缀信息
         * @desc : 在生成诊断信息时，用于填写前缀字段的预制信息 */
        string mprefix;

    public:
        /**
         * @operator () : 生成诊断信息
         * @desc : 
         *  生成一条诊断信息存于容器
         * @param code : 错误码
         * @param args : 诊断参数
         * @return Diagnostics& : 返回自身引用
         */
        template<typename ...Args>
        Diagnostics& operator () ( string code, Args&&... args ) {
            construct(-1, mprefix, code, std::forward<Args>(args)...);
            return *this;
        }

        Diagnostics& operator [] ( const string& prefix ) {
            mprefix = prefix;
            return *this;
        }

        Diagnostic& operator [] ( int index ) {
            return (*(chainz*)this)[index];
        }

        string prefix() {return mprefix;}
};

/**
 * @struct Diagnostic : 诊断信息
 * @desc :
 *  表示诊断信息对象的结构体，诊断信息自身不携带前缀信息
 */
struct Diagnostic {

    /**
     * @member prefix : 前缀信息
     * @desc :
     *  前缀信息一般用于指示诊断信息涉及的文档的前缀，也可以利用它传递其他信息 */
    string prefix;

    /**
     * @member code : 错误码
     * @desc : 编译器使用错误码选择如何组织诊断参数 */
    string code;

    /**
     * @member args : 诊断参数
     * @desc : 诊断信息模板中插入的参数 */
    tokens args;

    /**
     * @member info : 辅助信息
     * @desc : 为诊断信息添加的辅助信息 */
    Diagnostics info;

    /**
     * @ctor : 构造方法
     * @desc :
     *  构造方法用于方便地构造一条诊断信息
     * @param _prefix : 前缀信息
     * @param _code : 错误代码
     * @param _args : 诊断参数
     */
    template<typename ...Args>
    Diagnostic( string _prefix, string _code, Args&&... _args ):prefix(_prefix),code(_code) {
        (args.construct(-1,forward<Args>(_args)), ...);
    }

    Diagnostic( const Diagnostic& ) = default;
    Diagnostic( Diagnostic&& ) = default;

    /**
     * @operator () : 填写辅助信息
     * @desc :
     *  用于填写辅助信息的运算符
     * @param _prefix : 辅助信息的前缀信息
     * @param _code : 辅助信息的错误代码
     * @param _args : 辅助信息的诊断变量
     * @return Diagnostic& : 返回自身引用
     */
    template<typename ...Args>
    Diagnostic& operator () ( string _prefix, string _code, Args&&... _args ) {
        info.construct(-1, _prefix, _code, std::forward<Args>(_args)... );
        return *this;
    }
};

/**
 * @struct DiagnosticTemplate : 诊断信息模板
 * @desc :
 *  诊断信息模板描述了一个诊断信息结构体如何被处理成为一个诊断信息字符春
 */
struct DiagnosticTemplate {

    /**
     * @member severity : 严重性
     * @desc : 描述诊断信息的严重性,有效取值为1~4 */
    int severity;

    /**
     * @member begin : 起始行列
     * @desc :
     *  描述如何从诊断变量中找出起始行列
     *  <0>: 界定方式:
     *      < 0 --- 取变量末尾位置
     *      = 0 --- 无起始位置
     *      > 0 --- 取变量起始位置
     *  <1>: 变量下标 */
    tuple<int,int> beg;

    /**
     * @member begin : 终止行列
     * @desc :
     *  描述如何从诊断变量中找出终止行列
     *  <0>: 界定方式:
     *      < 0 --- 取变量末尾位置
     *      = 0 --- 无终止位置
     *      > 0 --- 取变量起始位置
     *  <1>: 变量下标 */
    tuple<int,int> end;

    /**
     * @member msg : 信息格式
     * @desc :
     *  描述如何将诊断变量组织成为诊断信息的格式字符串 */
    string msg;
};

/**
 * @struct DiagnosticLanguage : 诊断语种
 * @desc :
 *  诊断语种主要由诊断模板构成，一般从配置文件读取而来
 */
struct DiagnosticLanguage {

    /**
     * @member severities : 严重性模板
     * @desc : 对全部严重性的打印模板 */
    string severities[4];

    /**
     * @member templates : 诊断模板表
     * @desc : 当前语种的诊断信息模板表 */
    map<string,DiagnosticTemplate> templates;
};

/**
 * @class DiagnosticEngine : 诊断引擎
 * @desc :
 *  诊断引擎用于根据配置管理诊断信息
 *  提供诊断信息打印功能等
 */
class DiagnosticEngine {

    private:
        string format;
        const DiagnosticLanguage* language;
        map<string,DiagnosticLanguage> languages;

    public:

        /**
         * @ctor : 构造器
         * @desc :
         *  构造器会为诊断引擎配备基础诊断模板
         */
        DiagnosticEngine();

        /**
         * @method configure : 配置
         * @desc :
         *  使用json对象对诊断引擎进行配置
         * @param config : 配置信息
         * @return bool : 配置是否成功
         */
        bool configure( const json& config );

        /**
         * @method configureFormat : 配置格式
         * @desc :
         *  设置打印诊断信息的格式
         * @param fmt : 格式字符串
         */
        void configureFormat( const string& fmt );

        /**
         * @method configureLanguage : 配置某种语言
         * @desc :
         *  使用此方法配置某种语言，此方法会首先重置此语言
         * @param language : 语言名称
         * @param config : 配置信息，对应于总配置信息中language对象中的成员
         */
        bool configureLanguage( const string& language, const json& config );

        /**
         * @method enumerateLanguages : 枚举所有语言
         * @desc :
         *  枚举配置成功的所有语言
         * @return chainz<string> : 语言序列
         */
        chainz<string> enumerateLanguages() const;

        /**
         * @method selectLanguage : 选择语言
         * @desc :
         *  选择一种语言作为当前语言，打印之前的必备步骤
         */
        bool selectLanguage( const string& lang );

        /**
         * @method printToString : 打印到字符串
         * @desc :
         *  将诊断信息打印到字符串
         * @return string : 格式化的诊断信息字符串
         */
        string printToString( const Diagnostic& d )const;

        /**
         * @method printToString : 打印到字符串
         * @desc :
         *  将诊断信息打印到字符串
         * @param s : 诊断信息集
         * @return chainz<string> : 格式化的诊断信息字符串序列
         */
        chainz<string> printToString( const Diagnostics& s )const;

        /**
         * @method printToJson : 打印到Json
         * @desc :
         *  将诊断信息打印到JSON
         * @param d : 诊断信息
         * @return json : 诊断报告对象
         */
        json printToJson( const Diagnostic& d )const;

        /**
         * @method printToJson : 打印到Json
         * @desc :
         *  将诊断信息打印到JSON
         * @param s : 诊断信息集合
         * @return josn : 诊断报告对象数组
         */
        json printToJson( const Diagnostics& s )const;

    private:
        /**
         * @method organizeDiagnosticInformation : 组织诊断信息
         * @desc :
         *  将诊断信息所携带的诊断变量插入诊断信息模板，组成字符串
         * @param d : 诊断信息
         * @param colored : 是否着色
         * @return string : 未格式化的诊断信息
         */
        string organizeDiagnosticInformation( const Diagnostic& d, bool colored = true ) const;

};

}

#endif