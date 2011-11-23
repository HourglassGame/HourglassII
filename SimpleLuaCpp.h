#ifndef HG_SIMPLE_LUA_CPP_H
#define HG_SIMPLE_LUA_CPP_H
#include "lua/lua.h"
#include <cstdio>
#include <boost/swap.hpp>
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
    LuaState(LuaState&& other) :
        ptr(0)
    {
        swap(other);
    }
    LuaState& operator=(LuaState&& other)
    {
        swap(other);
        return *this;
    }
    void swap(LuaState& other) {
        boost::swap(ptr, other.ptr);
    }
    ~LuaState();
    lua_State* ptr;
};

inline void swap(LuaState& l, LuaState& r) {
    return l.swap(r);
}
}

#endif //HG_SIMPLE_LUA_CPP_H
