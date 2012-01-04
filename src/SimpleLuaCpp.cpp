#include "SimpleLuaCpp.h"

#include "multi_thread_allocator.h"
#include "lua/lauxlib.h"
#include <new>
#include <cassert>
namespace hg {
namespace {
    void* multi_thread_luaalloc (
        void* ud,
        void* ptr,
        size_t osize,
        size_t nsize)
    {
        (void)ud;  (void)osize;  /* not used */
        if (nsize == 0) {
            multi_thread_free(ptr);
            return 0;
        }
        else {
            return multi_thread_realloc(ptr, nsize);
        }
    }
}
LuaState::LuaState()
    : ptr(0)
{
}
LuaState::LuaState(new_state_t) 
    : ptr(lua_newstate(multi_thread_luaalloc, 0))
{
    if (ptr) {
        //TODO, change to a proper panic function!
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
