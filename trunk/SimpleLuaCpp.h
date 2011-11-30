#ifndef HG_SIMPLE_LUA_CPP_H
#define HG_SIMPLE_LUA_CPP_H
#include "lua/lua.h"
#include <cstdio>
#include <boost/swap.hpp>
#include <boost/move/move.hpp>
#include <cassert>
namespace hg {
inline int panic (lua_State* L) {
    fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
                   lua_tostring(L, -1));
    assert(false);
    return 0;
}

//RAII class for lua_State.
//Movable but non-copyable, as I do not know
//any way to copy a lua_State.
struct LuaState {
    struct new_state_t {};
    LuaState();
    explicit LuaState(new_state_t);
    LuaState(BOOST_RV_REF(LuaState) other) :
        ptr(0)
    {
        swap(other);
    }
    LuaState& operator=(BOOST_RV_REF(LuaState) other)
    {
        swap(other);
        return *this;
    }
    void swap(LuaState& other) {
        boost::swap(ptr, other.ptr);
    }
    ~LuaState();
    lua_State* ptr;
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(LuaState)
};

inline void swap(LuaState& l, LuaState& r) {
    return l.swap(r);
}
}//namespace hg

#endif //HG_SIMPLE_LUA_CPP_H
