#ifndef HG_SIMPLE_LUA_CPP_H
#define HG_SIMPLE_LUA_CPP_H
#include "lua/lua.h"
#include <boost/swap.hpp>
#include <boost/move/move.hpp>
#include <boost/exception/enable_current_exception.hpp>
#include "unique_ptr.h"
#include "LuaError.h"
namespace hg {
inline int panic (lua_State* L) {
    //Check whether this is a memory allocation error
    void* allocator_ud;
    lua_getallocf(L, &allocator_ud);
    bool is_oom(*static_cast<bool*>(allocator_ud));
    if (is_oom) {
        throw std::bad_alloc();
    }
    else {
        throw boost::enable_current_exception(LuaError(L));
    }
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
        is_oom(new bool(false)), ptr(0)
    {
        swap(other);
    }
    LuaState& operator=(BOOST_RV_REF(LuaState) other)
    {
        swap(other);
        return *this;
    }
    void swap(LuaState& other) {
        boost::swap(is_oom, other.is_oom);
        boost::swap(ptr, other.ptr);
    }
    ~LuaState();
    unique_ptr<bool> is_oom;
    lua_State* ptr;
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(LuaState)
};

inline void swap(LuaState& l, LuaState& r) {
    return l.swap(r);
}
}//namespace hg

#endif //HG_SIMPLE_LUA_CPP_H
