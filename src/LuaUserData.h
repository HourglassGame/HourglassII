#ifndef HG_LUA_USER_DATA_H
#define HG_LUA_USER_DATA_H
#include <tbb/atomic.h>
#include "lua/lua.h"
namespace hg {
//This is the type used for the userdata which is attached to
//the allocator. This happens to also be useful for holding other
//information, so it has been hacked to hold the information that you see here.
struct LuaUserData {
    bool is_out_of_memory;
    tbb::atomic<bool> is_interrupted;
};
inline LuaUserData& getUserData(lua_State* L) {
    void* ud;
    lua_getallocf(L, &ud);
    return *static_cast<LuaUserData*>(ud);
}
}
#endif
