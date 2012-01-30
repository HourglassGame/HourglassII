#include "SimpleLuaCpp.h"

#include "multi_thread_allocator.h"
#include "lua/lauxlib.h"
#include <new>
#include <cassert>
#include <limits>
namespace hg {
namespace {
    void* multi_thread_luaalloc (
        void* ud,
        void* ptr,
        size_t osize,
        size_t nsize)
    {
        bool& is_oom(*static_cast<bool*>(ud));
        if (nsize == 0) {
            multi_thread_free(ptr);
            return 0;
        }
        else {
            if (osize >= nsize) {
                void* p(multi_thread_realloc(ptr, nsize));
                return p ? p : ptr;
            }
            if (is_oom) return 0;
            void* p(multi_thread_realloc(ptr, nsize));
            is_oom = !p;
            return p;
        }
    }
}
LuaState::LuaState()
    : is_oom(), ptr(0)
{
}
LuaState::LuaState(new_state_t) 
    : is_oom(new bool(false)), ptr(lua_newstate(multi_thread_luaalloc, is_oom.get()))
{
    if (ptr) {
        lua_atpanic(ptr, &panic);
    }
    else {
        throw std::bad_alloc();
    }
}
LuaState::~LuaState()
{
    if (ptr) {
        lua_close(ptr);
    }
}
}
