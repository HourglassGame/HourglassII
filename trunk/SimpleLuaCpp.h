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

struct LuaState;
struct LuaStateMove_t
{
    LuaState& L_;
    explicit LuaStateMove_t(LuaState& L):
        L_(L)
    {}

    LuaState& operator*() const
    {
        return L_;
    }

    LuaState* operator->() const
    {
        return &L_;
    }
private:
    void operator=(LuaStateMove_t&);
};
//RAII class for lua_State.
//Moveable but non-copyable, as I do not know
//any way to copy a lua_State.
struct LuaState {
    struct new_state_t {};
    LuaState();
    LuaState(new_state_t);
    LuaState(LuaState& other) :
        ptr(0)
    {
        swap(other);
    }
    LuaState& operator=(LuaState& other)
    {
        swap(other);
        return *this;
    }
    LuaState(LuaStateMove_t mover) :
        ptr(0)
    {
        swap(*mover);
    }
    LuaState& operator=(LuaStateMove_t mover)
    {
        swap(*mover);
        return *this;
    }


    void swap(LuaState& other) {
        boost::swap(ptr, other.ptr);
    }
    LuaStateMove_t move() {
        return LuaStateMove_t(*this);
    }
    operator LuaStateMove_t() {
        return move();
    }
    ~LuaState();
    lua_State* ptr;
    private:
};
inline void swap(LuaState& l, LuaState& r) {
    return l.swap(r);
}
}

#endif //HG_SIMPLE_LUA_CPP_H
