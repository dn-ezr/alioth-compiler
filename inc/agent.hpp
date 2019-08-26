#ifndef __agent__
#define __agent__

#include "chainz.hpp"
#include "token.hpp"

namespace alioth {
template<typename T> class agent;

class thing {
    public:     token   phrase;
    protected:  int ref_count;
        virtual ~thing() {}
    public: 
        thing():ref_count(0){}
        thing( const thing& ) = default;
        thing(thing&&) = default;
        thing& operator = ( const thing& ) = default;
        thing& operator = ( thing&& ) = default;
        template<typename T>friend class agent;
};

template<typename T>
class agent {

    private:
        thing* p;

        thing* get(thing* stp)const {
            if( !stp ) return nullptr;
            stp->ref_count += 1;
            return stp;
        }
        void fre(thing* tp)const {
            if( tp ) {
                tp->ref_count -= 1;
                if( tp->ref_count <= 0 ) delete tp;
            }
        }
    public:
        agent( thing* t = nullptr):p(get(t)) {

        }
        agent( const agent& an ):p(get(an.p)) {

        }
        agent(agent&& an):p(an.p) {
            an.p = nullptr;
        }
        ~agent() {
            fre(p);
            p = nullptr;
        }

        agent& operator=(thing* t) {
            fre(p);
            p = get(t);
            return *this;
        }
        agent& operator=(const agent& an) {
            an.get(an.p);
            fre(p);
            p = an.p;
            return *this;
        }
        agent& operator=(agent&& an ) {
            if( p != an.p ) fre(p);
            p = an.p;
            an.p = nullptr;
            return *this;
        }

        bool operator==(T* t)const {
            return t == *this;
        }
        bool operator==(const agent& an)const {
            return an.p == p;
        }
        bool operator!=(T* t)const {
            return !(p == t);
        }
        bool operator!=(const agent& an)const {
            return an.p != p;
        }

        T* operator->()const {
            return dynamic_cast<T*>(p);
        }
        operator T*()const {
            return dynamic_cast<T*>(p);
        }
};

using anything = agent<thing>;
using everything = chainz<anything>;
}
#endif