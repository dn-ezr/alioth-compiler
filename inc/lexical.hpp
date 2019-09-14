#ifndef __lexical__
#define __lexical__

#include "alioth.hpp"
#include "token.hpp"
#include <iostream>

namespace alioth {
using namespace std;
/**
 * @class LexicalContext : 词法上下文
 * @desc :
 *  词法上下文用于从源代码数据流建立词法分析上下文
 *  词法分析的一系列步骤都是在词法分析上下文进行的操作
 */
class LexicalContext {

    private:
        /**
         * @member source : 源代码输入流
         * @desc : 指向源代码输入流的指针 */
        istream& source;

        /**
         * @member pre : 字符预览
         * @desc : 扫描过程中，每个决策开始之前，当前输入字节的预览 */
        int pre;

        /**
         * @member off : 偏移量
         * @desc : 此成员被用于“单词测试”和“前缀测试” */
        int off;

        /**
         * @member begl,begc : 当前源代码行列 
         * @desc: 在词法分析过程中跟踪记录当前行列，此二者被用来定位词法符号 */
        int begl, begc;

        /**
         * @member target : 目标状态
         * @desc : 在前缀测试中,若前缀匹配成功,跳转的目标状态值 */
        int target;

        /**
         * @member fixed : 确定样本
         * @desc : 在单词测试和前缀测试中使用的样本 */
        string fixed;

        /**
         * @member state : 当前状态
         * @desc : 词法分析有限状态机的当前状态 */
        int state;

        /**
         * @member stay : 停滞口令
         * @desc : 若此值为true,有限状态机停滞一个字节,再决策一次 */
        bool stay;

        /**
         * @member T : 临时记号
         * @desc : 词法分析过程中,正在构建的符号 */
        token T;

        /**
         * @member hit : 命中符号
         * @desc : 当"赋值测试"命中时,用于替换T.id的终结符类型 */
        int hit;

        /**
         * @member synst : 语法分析器状态
         * @desc : 词法分析器内置微型语法分析器的状态 */
        int synst;

        /**
         * @member ret : 词法分析产物
         * @desc : 每次词法分析时用作产物的词法符号序列 */
        tokens ret;

        /**
         * @member heading : 限制
         * @desc : 此成员描述词法分析流程是否受到范围限制，若受到范围限制,则词法分析仅分析模块签名 */
        bool heading;
    
    public:

        /**
         * @ctor : 构造函数
         * @desc :
         *  构造一个词法分析上下文
         * @param is : 源代码输入流
         * @param limit : 描述词法分析是否受到范围限制，若此值为真，则词法分析仅检测有效的模块签名
         */
        LexicalContext( istream& is, bool limit = false );

        /**
         * @method perform: 执行
         * @desc :
         *  执行词法分析，获取词法序列
         */
        tokens perform();

    private:
        /**
         * @method goon : 继续
         * @desc :
         *  考虑停滞口令后,输入一个字节,继续检查下一个字节
         *  若停滞口令有效,则失活停滞口令,并结束
         */
        void goon();

        /**
         * @method check : 确认输入
         * @desc :
         *  确认当前正在构建的词法符号已经完成,将其输入分析产物序列
         *  同时,考虑词法分析范围限制,使用微型语法分析算法做出状态决策
         * @param t : 确认的词法符号
         * @param s : 是否连同当前正在检查的预览字节一起确认输入
         */
        void check( int t, bool s );

        /**
         * @method test : 开始一次单词测试
         * @desc :
         *  单词测试用于测试接下来的输入内容是否与词汇表中的指定单词的书写形式相同
         *  由当前已分析的前缀最多仅能引导一种确定词汇时,使用单词测试简化分析流程
         *  单词测试流程由词法分析算法包含
         *  此方法用于进入单词测试流程
         * @param v : 要测试的词法符号
         * @param o : 开始测试时已经扫描的前缀的偏移量
         */
        void test( int v, int o );

        /**
         * @method prefix : 开始一次前缀测试
         * @desc :
         *  前缀测试用于测试输入序列是否满足指定前缀,若满足,则跳转到指定状态进行后续处理
         *  前缀测试用于应对前缀相同的多个不同词汇的集中扫描处理.
         *  此方法用于开始一次前缀测试
         * @param s : 前缀测试样板
         * @param o : 前缀测试开始的偏移量
         * @param t : 前缀测试通过后应当跳转的状态
         */
        void prefix( const char* s, int o, int t );

        /**
         * @method assign : 开始一次赋值测试
         * @desc :
         *  当已分析的内容足以成为运算符但也可能与'='联结成为赋值运算符,使用赋值测试来统一处理逻辑,消除重复代码.
         *  此方法用于进入赋值测试所需的状态.
         * @param h : 当赋值测试命中时,成立的词法符号
         * @param m : 当赋值测试未通过时,成立的词法符号
         */
        void assign( int h, int m );

        /**
         * @method sequence : 开始一个序列扫描
         * @desc :
         *  序列扫描用于扫描由引号包含的,表示文本的内容
         * @param h : 当扫描命中时,成立的词法符号
         */
        void sequence( int h );

        /**
         * @method islabelb : 判断字符是否能用作标识符的开头
         * @desc :
         *  此方法用于判断字节是否能用作标识符的开头
         * @param c : 要判断的字符
         * @return bool
         */
        static bool islabelb( int c );

        /**
         * @method islabel : 判断字符是否可以用作标识符的内容
         * @desc :
         *  此方法用于判断字符是否可用作标识符的内容
         * @param c : 要判断的字符
         * @return bool
         */
        static bool islabel( int c );
};

}

#endif