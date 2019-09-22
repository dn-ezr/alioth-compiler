#ifndef __ptree__
#define __ptree__

#include <map>
/**
 * @class ptree : prefix tree 前缀树
 * @desc :
 *  用于存储大量以字符串为键的数据，当键存在大量前缀时，节省空间。
 */
template<typename T>
class ptree {
    private:
        /**
         * @member parent : 父节点
         * @desc : 父节点指针 */
        ptree* parent;

        /**
         * @member children : 子节点
         * @desc : 使用map避免On查找 */
        std::map<char,ptree> children;

        /**
         * @member data : 数据
         * @desc : 数据实体 */
        T* data;

    public:
        ptree( ptree* p = nullptr ):
            parent(p),data(nullptr){}
        ptree( T&& _data, ptree * p ):
            parent(p),data(new T(std::move(_data))){}
        ptree( const T& _data, ptree * p ):
            parent(p),data(new T(_data)){}

        ~ptree(){
            if( data ) delete data;
        }

        ptree& operator[] ( const char* index ) {
            if( index == nullptr ) throw std::invalid_argument("ptree::operator []( char* index): invalid null pointer for index.");
            if( *index == 0 ) return *this;
            if( !children.count(index[0]) ) {
                auto& ch = children[index++[0]];
                ch.parent = this;
                return ch[index];
            }
            return children[index++[0]][index];
        }

        ptree* at( const char* index ) {
            if( index == nullptr ) throw std::invalid_argument("ptree::at( char* index): invalid null pointer for index.");
            if( *index == 0 ) return this;
            if( !children.count(index[0]) ) return nullptr;
            return children[index++[0]].at(index);
        }

        ptree* far( const char* index ) {
            if( index == nullptr ) throw std::invalid_argument("ptree::at( char* index): invalid null pointer for index.");
            if( *index == 0 ) return this;
            if( !children.count(index[0]) ) 
                if( children.size() ) return nullptr;
                else return this;
            return children[index++[0]].far(index);
        }

        void store( T&& d ) {
            if( data ) *data = d;
            else data = new T(std::move(d));
        }

        void store( const T& d ) {
            if( data ) *data = d;
            else data = new T(d);
        }

        T& get() {
            if( data ) return *data;
            throw std::runtime_error("ptree::get(): no data stored here.");
        }

        T* getp() {
            return data;
        }
};

#endif