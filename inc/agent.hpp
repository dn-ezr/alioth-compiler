#ifndef __agent__
#define __agent__

#include "chainz.hpp"
#include "token.hpp"

namespace alioth {
template<typename T> class agent;

class basic_thing {
    protected:  int ref_count;
        virtual ~basic_thing() {}
    public: 
        basic_thing():ref_count(0){}
        basic_thing(const basic_thing&):ref_count(0){}
        basic_thing(basic_thing&&):ref_count(0){}
        basic_thing& operator = ( const basic_thing& ){return *this;}
        basic_thing& operator = ( basic_thing&& ){return *this;}
        template<typename T>friend class agent;
};

class thing : public basic_thing {
    public:     token   phrase;
    template<typename T>friend class agent;

    thing() = default;
    thing(const thing&) = default;
    thing(thing&&) = default;
    thing& operator = ( const thing& ) = default;
    thing& operator = ( thing&& ) = default;
};

template<typename T>
class agent {

    private:
        basic_thing* p;

        basic_thing* get(basic_thing* stp)const {
            if( !stp ) return nullptr;
            stp->ref_count += 1;
            return stp;
        }
        void fre(basic_thing* tp)const {
            if( tp ) {
                tp->ref_count -= 1;
                if( tp->ref_count <= 0 ) delete tp;
            }
        }
    public:
        agent( basic_thing* t = nullptr):p(get(t)) {

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

        agent& operator=(basic_thing* t) {
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

using anything = agent<basic_thing>;
using everything = chainz<anything>;
const everything nothing = {};
#define something(x) struct x;using $##x = agent<x>
}
#endif