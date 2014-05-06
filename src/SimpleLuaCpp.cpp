#include "SimpleLuaCpp.h"

#include "multi_thread_allocator.h"
#include "lua/lauxlib.h"
#include <new>
#include <cassert>
#include <limits>
namespace hg {
namespace {
    void *multi_thread_luaalloc (
        void *ud,
        void *ptr,
        size_t osize,
        size_t nsize)
    {
        LuaUserData &user_data(*static_cast<LuaUserData*>(ud));
        if (nsize == 0) {
            multi_thread_free(ptr);
            return 0;
        }
        else {
            if (osize >= nsize) {
                void *p(multi_thread_realloc(ptr, nsize));
                return p ? p : ptr;
            }
            if (user_data.is_out_of_memory()) return 0;
            void *p(multi_thread_realloc(ptr, nsize));
            user_data.set_out_of_memory(!p);
            return p;
        }
    }
}
LuaState::LuaState()
    : ud(), ptr(nullptr)
{
}
LuaState::LuaState(new_state_t)
    : ud(hg::make_unique<LuaUserData>()), ptr(lua_newstate(multi_thread_luaalloc, ud.get()))
{
    if (ptr) {
        lua_atpanic(ptr, &panic);
    }
    else {
        throw std::bad_alloc();
    }
}
LuaState::~LuaState() noexcept
{
    if (ptr) {
        lua_close(ptr);
    }
}
}
