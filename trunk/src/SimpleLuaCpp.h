#ifndef HG_SIMPLE_LUA_CPP_H
#define HG_SIMPLE_LUA_CPP_H
#include "lua/lua.h"
#include <boost/swap.hpp>
#include <boost/move/move.hpp>
#include <boost/exception/enable_current_exception.hpp>
#include "unique_ptr.h"
#include "LuaError.h"
#include "LuaUserData.h"
#include "OperationInterruptedException.h"
#include <iostream> 
namespace hg {
inline int panic (lua_State* L) {
    //Check whether this is a memory allocation error
    LuaUserData& ud(getUserData(L));
    if (ud.is_out_of_memory) {
        throw std::bad_alloc();
    }
    else if (ud.is_interrupted) {
        std::cerr << "Lua Interrupted\n";
        throw boost::enable_current_exception(OperationInterruptedException());
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
    LuaState(BOOST_RV_REF(LuaState) o) :
        is_oom(new LuaUserData()), ptr(0)
    {
        swap(o);
    }
    LuaState& operator=(BOOST_RV_REF(LuaState) o)
    {
        swap(o);
        return *this;
    }
    void swap(LuaState& o) {
        boost::swap(is_oom, o.is_oom);
        boost::swap(ptr, o.ptr);
    }
    ~LuaState();
    unique_ptr<LuaUserData> is_oom;
    lua_State* ptr;
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(LuaState)
};

inline void swap(LuaState& l, LuaState& r) {
    return l.swap(r);
}
}//namespace hg

#endif //HG_SIMPLE_LUA_CPP_H
