#ifndef __chainz__
#define __chainz__

/**
 * 作者:ezr
 * 最后更新 : 2019/11/8
 * 
 * 添加initializer_list支持
 * 
 * 修正了remove方法传入负数,反向寻址时抛出溢出异常的bug
 * 修正了迭代器后缀自变运算依然提前变化的bug
 * 
 * 
 * chainz使用指针池结构存储数据
 * 用以弥补链式寻址开销大的缺陷
 * 代价则是内存碎片化极为严重
 * 
 * 到目前为止,chainz被添加了很多功能
 * 
 * chainz支持基于范围的迭代
 * 支持嵌套结构的层次提升(即 ele = ele[x]这样的行为)
 * chainz能反向寻址(即 ele[-1] 表示ele的倒数第一个元素这样的寻址)
 * chainz为自己和存入其中的元素都提供了移动支持(即充分使用移动构造函数来减小开销)
 * 
 */

#include <functional>
#include <cstring>
#include <iterator>
#include <initializer_list>

#define OUT     //标记一个参数是输出参数
#define OPT     //标记一个参数是可选的参数
#define IN      //标记一个参数是输入参数

template<typename T>
class chainz {

    protected:
        /**
         * 定义迭代器类以支持range-based for特性
         * 由于需要对const和非const两种内容实现同一风格的迭代器,迭代器使用模板类声明
         * ---- aC表示chainz的引用的类型
         * ---- aT则表示chainz的内容的类型
         */
        template<typename aC,typename aT>
        struct b_iterator:public std::iterator<std::random_access_iterator_tag,aT,int> {

            public:
                
                aC& r;      //迭代器所对应的容器引用
                int pos;    //迭代器当前所指示的位置

            public:
                /**
                 * @param _r: 迭代器所对应容器的引用
                 * @param p: 迭代器当前所指向的位置,迭代器不对数字进行修正
                 * ---- 这意味着两点:
                 * ---- ---- 1:正数和负数对迭代器来说是不同的两个位置,即使实际上在chainz中指向同一个位置
                 * ---- ---- 2:若在迭代过程中,容器的内容数量有所变化,迭代器的数值不会随之变化
                 * @state: 构造函数
                 */
                b_iterator( aC& _r, int p ):r(_r),pos(p){}
                /**
                 * @param i: 另一个迭代器
                 * @state: 拷贝构造函数
                 */
                b_iterator( const b_iterator& i ):r(i.r),pos(i.pos){}

                /**
                 * @state: 析构函数
                 */
                ~b_iterator(){}

                /**
                 * @return: 返回自身引用
                 * @state: 自曾函数,将迭代器向前(next方向)推移一个单位
                 */
                b_iterator& operator++(){pos += 1;return *this;}
                b_iterator operator++(int){b_iterator an = *this;pos += 1;return an;}

                /**
                 * @return: 返回自身引用
                 * @state: 自减函数,将迭代器向后(prev方向)推移一个单位
                 */
                b_iterator& operator--(){pos -= 1;return *this;}
                b_iterator operator--(int){b_iterator an = *this;pos -= 1;return an;}

                /**
                 * @param a: 向前推进单位
                 * @return: 返回产生的新迭代器
                 * @state: 向前推进函数
                 */
                b_iterator operator+( int a ){return b_iterator( r, pos+a );}

                /**
                 * @param s: 向后推进单位
                 * @return: 返回产生的新迭代器
                 * @state: 向后推进函数
                 */
                b_iterator operator-( int s ){return b_iterator( r, pos-s );}

                /**
                 * @param a: 向前推进的单位
                 * @return: 返回自身引用
                 * @state: 向前推进函数
                 */
                b_iterator& operator+=( int a ){pos+=a;return *this;}

                /**
                 * @param s: 向后推进的单位
                 * @return: 返回自身引用
                 * @state: 向后推进函数
                 */
                b_iterator& operator-=( int s ){pos-=s;return *this;}

                /**
                 * @return: 容器中迭代器对应位置上的元素引用
                 * @state: 间接引用方法,若迭代器所指向的位置超出了容器的范围,容器会抛出异常
                 */
                aT& operator*(){return r[pos];}
                aT* operator->(){return r.get(pos);}

                /**
                 * @param an: 另一个迭代器
                 * @return: 两迭代器所指容器与位置是否相同
                 * @state: 判断两个迭代器所指是否相同
                 */
                bool operator==( const b_iterator& an ){ return &r == &an.r and pos == an.pos;}

                /**
                 * @param an: 另一个迭代器
                 * @return: 两迭代器所指容器与位置是否不等
                 * @state: 判断两迭代器所指是否不等
                 */
                bool operator!=( const b_iterator& an ){ return !(*this==an);}
        };
    public:
        /**
         * 定义两种迭代器,分别应对通用的实例和常量实例
         */
        using iterator = b_iterator<chainz,T>;
        using citerator = b_iterator<const chainz,const T>;

    protected:

        /**
         * chainz的内容是元素的指针的列表
         * 所以将元素指针视为一种独立的类型
         */
        using R = T*;
        
        R*      m_pool;     //元素指针列表
        int     m_count;    //元素总量统计
    
    public:
        /**
         * @state: 构造函数
         */
        chainz():
        m_pool(nullptr),m_count(0) {

        }

        chainz(std::initializer_list<T> list ):chainz() {
            for( auto e : list ) construct(-1, e );
        }

        /**
         * @param another: 另一个chainz实例
         * @state: 拷贝构造函数,从另一个实例按照原顺序拷贝每一个元素
         */
        chainz( const chainz& another ):
        m_pool(nullptr),m_count(0) {
            for( int i = 0; i < another.m_count; i++ )
                push( another[i] );
        }

        /**
         * @param temp: 一个右值实例
         * @state: 移动构造函数,从右值实例中抓取系统资源
         */
        chainz( chainz&& temp ):
        m_pool(temp.m_pool),m_count(temp.m_count) {
            temp.m_pool = nullptr;
            temp.m_count = 0;
        }

        /**
         * @param data : 首个元素
         * @param args : 参数包
         */
        template<typename ...Args>
        chainz( T&& data, Args&&... args ):chainz() {
            construct(-1,std::forward<T>(data)) , ( construct(-1,std::forward<Args>(args)) , ... );
        }

        /**
         * @state: 析构函数
         */
        ~chainz() {
            clear();
        }

        /**
         * @param another: 另一个chainz实例
         * @return: 自身引用
         * @state: 赋值运算,将另一个chainz实例的快照装入
         * ---- 容器原有数据会被清空
         */
        chainz& operator=( const chainz& another ) {
            chainz temp = another;
            *this = std::move(temp);
            return *this;
        }

        /**
         * @param temp: 一个右值实例
         * @return: 自身引用
         * @state: 赋值运算,从右值实例中移动数据
         * ---- 容器原有数据会被清空
         */
        chainz& operator=( chainz&& temp ) {
            chainz tempz = std::move(temp);
            clear();
            m_pool = tempz.m_pool;
            m_count = tempz.m_count;
            tempz.m_count = 0;
            tempz.m_pool = nullptr;
            return *this;
        }

        /**
         * @return: 容器内容单位总量
         * @state: 统计容器中内容单位的总量
         */
        int size()const{
            return m_count;
        }

        /**
         * @param data: 插入数据的实例
         * @param index: 数据插入后,所在的位置
         * @return: 插入是否成功
         * @state: 插入方法,将data插入到容器中,使得插入后data的位置为index
         * ---- 若index >= 0
         * ---- ---- 则,index的范围是 [0,size()],代表了一个从0开始的正向地址
         * ---- 若index < 0
         * ---- ---- 则,index的范围是 [-1,-1-size()],代表了一个从size()开始的负向地址
         */
        bool insert( IN const T& data, IN int index ) {
            T dat(data);
            return insert(std::move(dat),index);
        }
        bool insert( IN T&& data, IN int index ) {

            //若表为空,则情况单独处理
            if( m_count == 0 ) {
                if( index == 0 or index == -1 ) {
                    m_pool = new R[1];
                    m_pool[0] = new T(std::move(data));
                } else {
                    return false;
                }
            
            //若表不为空
            } else {
                //若索引小于0,则先修正索引到正向
                if( index < 0 )
                    index = m_count + 1 + index;
                //若正向索引超出范围,则返回失败
                if( index > m_count or index < 0 )
                    return false;
                
                //创建新表,将原表中的元素指针映射到新表中,同时插入新的元素的指针
                R* nr = new R[m_count+1];
                memcpy( nr, m_pool, index*sizeof(R) );
                nr[index] = new T(std::move(data));
                memcpy( nr+index+1, m_pool+index, (m_count-index)*sizeof(R));

                //删除原表,替换表指针
                delete[] m_pool;
                m_pool = nr;
            }

            //收尾工作
            m_count += 1;
            return true;
        }
        bool insert( IN T* pdata, IN int index ) {

            //若表为空,则情况单独处理
            if( m_count == 0 ) {
                if( index == 0 or index == -1 ) {
                    m_pool = new R[1];
                    m_pool[0] = pdata;
                } else {
                    return false;
                }
            
            //若表不为空
            } else {
                //若索引小于0,则先修正索引到正向
                if( index < 0 )
                    index = m_count + 1 + index;
                //若正向索引超出范围,则抛出异常
                if( index > m_count or index < 0 )
                    return false;
                
                //创建新表,将原表中的元素指针映射到新表中,同时插入新的元素的指针
                R* nr = new R[m_count+1];
                memcpy( nr, m_pool, index*sizeof(R) );
                nr[index] = pdata;
                memcpy( nr+index+1, m_pool+index, (m_count-index)*sizeof(R));

                //删除原表,替换表指针
                delete[] m_pool;
                m_pool = nr;
            }

            //收尾工作
            m_count += 1;
            return true;
        }

        /**
         * @param tag : 要取出的对象指针
         * @param index : 要取出的对象的索引
         * @state : 若对象在容器内,取出对象,交给调用者,否则无动作,返回空指针
         */
        T* pickout( IN T* tag ) {
            if( !tag ) return nullptr;
            for( int i = 0; i < m_count; i++ ) 
                if( m_pool[i] == tag ) return pickout(i);
            return nullptr;
        }
        T* pickout( int index ) {
            T* t = get(index);
            if( !t ) return nullptr;
            //若容器只有此元素一个,则执行清空动作
            if( m_count == 1 ) {
                delete[] m_pool;
                m_pool = nullptr;
                m_count = 0;
            
            //否则,应当重新映射指针表到新的表格中
            } else {
                R* nr = new R[m_count-1];
                memcpy( nr, m_pool, index*sizeof(R) );
                memcpy( nr+index, m_pool+index+1, (m_count-index-1)*sizeof(R) );
                delete[] m_pool;
                m_pool = nr;
                m_count -= 1;
            }
            return t;
        }

        /**
         * @param index: 创建实例的插入点
         * @param args: 对象实例的构造函数的参数
         */
        T& construct( IN int index ) {
            T* t = new T();
            if( insert(std::move(t),index) ) return *t;
            throw std::runtime_error("chainz::construct( int index ): fail to insert object");
        }
        template<typename ...Args>
        T& construct( IN int index, Args&&... args ){
            T* t = new T(std::forward<Args>(args)... );
            if( insert(std::move(t),index) ) return *t;
            throw std::runtime_error("chainz::construct( int index ): fail to insert object");
        }

        /**
         * @param index: 欲删除元素的位置
         * @return: 删除是否成功
         * @state: 删除index位置上的元素
         * ---- 其中,index可以小于零,相应规则同insert方法
         */
        bool remove( IN int index ) {
            if( index < 0 )
                index = m_count + index;
            if( index >= m_count or index < 0 )
                return false;
            
            //删除index对应位置上的元素
            delete m_pool[index];

            //若容器只有此元素一个,则执行清空动作
            if( m_count == 1 ) {
                delete[] m_pool;
                m_pool = nullptr;
                m_count = 0;
            
            //否则,应当重新映射指针表到新的表格中
            } else {
                R* nr = new R[m_count-1];
                memcpy( nr, m_pool, index*sizeof(R) );
                memcpy( nr+index, m_pool+index+1, (m_count-index-1)*sizeof(R) );
                delete[] m_pool;
                m_pool = nr;
                m_count -= 1;
            }

            //收尾
            return true;
        }

        /**
         * @param ref: 数据实例
         * @return: 删除是否成功
         * @state: 若ref实例在容器内,则删除ref实例
         */
        bool remover( IN const T& ref ) {

            //遍历地址表,判断实例是否属于容器
            for( int i = 0; i < m_count; i++ ) 
                if( m_pool[i] == &ref )
                    return remove(i);
            
            //若执行到此,说明实例不在容器内
            return false;
        }

        /**
         * @return: 无
         * @state: 清空容器,删除所有内容
         */
        void clear() {
            if( m_count != 0 ) {
                for( int i = 0; i < m_count; i++ )
                    if( m_pool[i] ) delete m_pool[i];
                delete[] m_pool;
                m_pool = nullptr;
                m_count = 0;
            }
            return;
        }

        /**
         * @param data: 将要入栈的数据实例
         * @return: 自身实例
         * @state: 将容器视为栈,在栈顶插入新的数据
         */
        bool push( IN const T& data ) {
            return insert( data, -1 );
        }
        bool push( IN T&& data ) {
            return insert(std::move(data), -1 );
        }

        /**
         * @return: 是否成功
         * @state: 将容器视为栈,从栈顶删除一个单位
         */
        bool pop() {
            return remove(-1);
        }

        /**
         * @param data: 用于接收数据的实例
         * @return: 是否成功
         * @state: 将容器视为栈,从栈顶移出一个单位
         */
        bool pop( OUT T& data ){
            T* p = get(-1);
            if( p == nullptr )
                return false;
            data = *p;
            return pop();
        }

        /**
         * @param data: 即将入队的数据实例
         * @return: 自身实例或操作是否成功
         * @state: 将容器视为队列,在队尾插入数据
         */
        bool inqueue( IN const T& data ) {
            return insert( data, -1 );
        }
        bool inqueue( IN T&& data ) {
            return insert(std::move(data), -1 );
        }
        chainz& operator<<( IN const T& data ) {
            inqueue(data);
            return *this;
        }
        chainz& operator<<(IN T&& data ) {
            inqueue(data);
            return *this;
        }

        /**
         * @return: 操作是否成功
         * @state: 将容器视为队列,从队头删除一个实例
         */
        bool outqueue(){
            return remove(0);
        }

        /**
         * @peram data: 用于接受数据的实例
         * @return: 操作是否成功
         * @state: 将容器视为队列,从队头移出一个单位
         */
        bool outqueue( OUT T& data ) {
            T* p = get(0);
            if( p == nullptr )
                return false;
            data = *p;
            return outqueue();
        }

        /**
         * @param index: 元素的索引
         * @return: 索引位置上,元素的指针
         * @state: 获取index位置上的元素的指针
         * ---- 若index位置上没有数据,则返回空指针
         */
        T* get( IN int index ) {
            if( index < 0 )
                index = m_count + index;    //与插入时不同,引用时容器内容总量不改变,所以不加一
            if( index >= m_count or index < 0 )
                return nullptr;
            else
                return m_pool[index];
        }
        const T* get( IN int index )const {
            if( index < 0 )
                index = m_count + index;
            if( index >= m_count or index < 0 )
                return nullptr;
            else
                return m_pool[index];
        }

        /**
         * @param index: 元素索引
         * @return: 索引位置上,元素的引用
         * @state: 获取index位置上的元素的引用
         * ---- 若index位置上没有数据,则抛出异常
         */
        T& operator[]( IN int index ) {
            T* p = get(index);
            if( !p ) throw std::out_of_range("chainz::operator[](int index): out of range");
            return *p;
        }
        const T& operator[]( IN int index )const {
            const T* p = get(index);
            if( !p ) throw std::out_of_range("chainz::operator[](int index)const: out of range");
            return *p;
        }

        /**
         * @param ref: 数据实例的引用
         * @return: ref实例在容器中的位置
         * @state: 若ref实例在容器中,则获取ref实例的索引值
         */
        int index( IN const T& ref ) const {
            for( int i = 0; i < m_count; i++ )
                if( &ref == m_pool[i] )
                    return i;
            return -1;
        }

        /**
         * @return: 迭代器
         * @state: 从不同环境创造迭代器
         * ---- 用以支持range-based-for
         */
        iterator begin(){
            return iterator(*this, 0);
        }
        citerator begin()const{
            return citerator(*this,0);
        }
        iterator end(){
            return iterator(*this,m_count);
        }
        citerator end()const{
            return citerator(*this,m_count);
        }

        /**
         * @param func: 筛选函数
         * @return: 筛选过的chainz容器
         * @state: 筛选方法
         * ---- func返回true时当前节点被添加到筛选容器中
         * ---- 遍历本容器,通过func筛选出一个新的链表返回之
         */
        chainz operator%( IN std::function<bool(const T&data)> func )const{
            chainz cnz = chainz();
            for( int i = 0; i < m_count; i++ )
                if( func( *(m_pool[i]) ) ) cnz << *(m_pool[i]);
            return std::move(cnz);
        }
        chainz map( IN std::function<bool(const T&data)> func )const {
            return operator%(func);
        }

        chainz operator+( const chainz& an )const {
            chainz res;
            int in = 0;
            res.m_count = an.m_count + m_count;
            if( res.m_count == 0 ) return std::move(res);
            res.m_pool = new R[res.m_count];
            for( const auto& i : *this ) res.m_pool[in++] = new T(i);
            for( const auto& i : an ) res.m_pool[in++] = new T(i);
            return std::move(res);
        }
        chainz operator+( chainz&& an )const {
            chainz res;
            int in = 0;
            res.m_count = an.m_count + m_count;
            if( res.m_count == 0 ) return std::move(res);
            res.m_pool = new R[res.m_count];
            for( const auto& i : *this ) res.m_pool[in++] = new T(i);
            memcpy(res.m_pool+in,an.m_pool,an.m_count*sizeof(R));
            delete an.m_pool;
            an.m_pool = nullptr;
            an.m_count = 0;
            return std::move(res);
        }
        chainz& operator+=( const chainz& an ) {
            int nc = m_count + an.m_count;
            int in = m_count;
            if( nc == 0 ) return *this;
            R* np = new R[nc];
            memcpy(np,m_pool,m_count*sizeof(R));
            for( const auto& i : an ) np[in++] = new T(i);
            delete m_pool;
            m_pool = np;
            m_count = nc;
            return *this;
        }
        chainz& operator+=( chainz&& an ) {
            int nc = m_count + an.m_count;
            if( nc == 0 ) return *this;
            R* np = new R[nc];
            memcpy(np,m_pool,m_count*sizeof(R));
            memcpy(np+m_count,an.m_pool,an.m_count*sizeof(R));
            delete an.m_pool;
            an.m_pool = nullptr;
            an.m_count = 0;
            delete m_pool;
            m_pool = np;
            m_count = nc;
            return *this;
        }
};

#endif