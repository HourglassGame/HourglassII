#ifndef HG_SIMPLE_LUA_CPP_H
#define HG_SIMPLE_LUA_CPP_H
#include "lua/lua.h"
#include <boost/swap.hpp>
#include "unique_ptr.h"
#include "LuaError.h"
#include "LuaUserData.h"
#include "OperationInterruptedException.h"
#include <iostream>
#include <thread>
#include <boost/throw_exception.hpp>
namespace hg {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4646) //[[noreturn]] declared with return type. Justified because `panic` has to be a lua_Cfunction, which returns int.
#endif
[[noreturn]] inline int panic(lua_State *L) {
    //Check whether this is a memory allocation error
    LuaUserData &ud(getUserData(L));
    if (ud.is_out_of_memory()) {
        throw std::bad_alloc();
    }
    else if (ud.is_interrupted()) {
        BOOST_THROW_EXCEPTION(OperationInterruptedException());
    }
    else {
        BOOST_THROW_EXCEPTION(LuaRunError(L));
    }
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//RAII class for lua_State.
//Movable but non-copyable, as I do not know
//any way to copy a lua_State.
struct LuaState {
    struct new_state_t {};
    LuaState();
    explicit LuaState(new_state_t);
    LuaState(LuaState &&o) noexcept :
        ud(), ptr()
    {
        swap(o);
    }
    LuaState &operator=(LuaState &&o) noexcept
    {
        swap(o);
        return *this;
    }
    void swap(LuaState &o) {
        boost::swap(ud, o.ud);
        boost::swap(ptr, o.ptr);
    }
    ~LuaState() noexcept;
    std::unique_ptr<LuaUserData> ud;
    lua_State *ptr;
};

inline void swap(LuaState &l, LuaState &r) {
    return l.swap(r);
}
}//namespace hg

#endif //HG_SIMPLE_LUA_CPP_H
