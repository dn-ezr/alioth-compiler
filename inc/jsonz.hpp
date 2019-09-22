#ifndef __json__
#define __json__

#include <istream>
#include <string>
#include <iterator>
#include <functional>
#include "chainz.hpp"

class json {
    public:
        enum type { boolean, integer, number, string, object, array, null };

    private:
        type mtype;
        void* mdata;
    
    protected:
        void clean();

    public:

        /**
         * @ctor : 构造方法
         * @desc :
         *  接受初始值构造json
         * @param type : 对于不同Json类型，初始化数据指针
         * @param long : 初始化整数类型
         * @param double : 初始化number类型
         * @param const std::string&, std::string&& : 初始化string类型
         * @param bool : 初始化boolean类型
         * @param const josn&, json&& : 拷贝和移动构造函数
         */
        json( type = null );
        json( long );
        json( double );
        json( const std::string& );
        json( std::string&& );
        json( bool );
        json( const json& );
        json( json&& );

        /**
         * @dtor : 析构方法
         * @desc :
         *  清空类型，释放数据指针
         */
        ~json();


        /**
         * @method is : 判断Json数据类型
         * @desc :
         *  判断Json数据类型
         */
        type is()const;
        bool is( type )const;

        /**
         * @operator bool,double,std::string : 强制类型转换运算符
         * @desc :
         *  进行强制类型转换,获得json保存的数据内容
         *  若类型不匹配,抛出异常
         */
        explicit operator bool&();
        explicit operator const bool&()const;

        explicit operator long&();
        explicit operator const long&()const;

        explicit operator double&();
        explicit operator const double&()const;

        operator std::string&();
        operator const std::string&()const;

        /**
         * @operator [std::string] : 键访问
         * @desc :
         *  以键访问Json对象中的属性,若类型不匹配抛出异常.
         *  若属性不存在,创建属性.
         *  若不允许创建不存在的属性,则抛出异常.
         * 类型不匹配抛出异常
         */
        json& operator[]( const std::string& );
        const json& operator[]( const std::string& )const;

        /**
         * @operator [int] : 下标访问
         * @desc :
         *  以下标访问Json数组的元素,支持以负数反向检索
         *  可修改的情况下,若访问下标超过容量,则用null填充扩展数组
         *  不可修改的情况下,抛出异常
         *  类型不匹配抛出异常
         */
        json& operator[]( int );
        const json& operator[]( int )const;

        /**
         * @method count : 统计对象或数组的容量
         * @desc :
         *  若类型不匹配抛出异常
         */
        int count( const std::string& )const;
        int count( const std::string&, type )const;
        int count()const;

        /**
         * @operator erase : 删除数组或对象的成员
         * @desc :
         *  若成员不存在,则无动作
         *  若类型不匹配,抛出异常
         */
        void erase( const std::string& );
        void erase( int );

        /**
         * @operator = : 赋值运算
         * @desc :
         *  赋值运算符会强制释放资源,改变数据类型
         *  若类型一致,不释放原数据指针
         */
        json& operator = ( bool );
        json& operator = ( long );
        json& operator = ( double );
        json& operator = ( const std::string& );
        json& operator = ( std::string&& );
        json& operator = ( const json& );
        json& operator = ( json&& );

        /**
         * @method FromJsonStream : 从输入流输入Json
         */
        static json FromJsonStream( std::istream& is );
        std::string toJsonString()const;
    
    public:
        chainz<json>::iterator begin();
        chainz<json>::citerator begin()const;
        chainz<json>::iterator end();
        chainz<json>::citerator end()const;

        int for_each( const std::function<bool(const std::string&, json&)>& fun );
        int for_each( const std::function<bool(const std::string&, const json&)>& fun ) const;
};

#endif