#ifndef HG_LUA_USER_DATA_H
#define HG_LUA_USER_DATA_H
#include <tbb/atomic.h>
#include "lua/lua.h"
#include <iostream>
#include <string>
namespace hg {
//This is the type used for the userdata which is attached to
//the allocator. This happens to also be useful for holding other
//information, so it has been hacked to hold the information that you see here.
struct LuaUserData {
    LuaUserData(): is_out_of_memory_(false), is_interrupted_() {
        is_interrupted_ = false;
    }
    
    void set_out_of_memory(bool value) {
        is_out_of_memory_ = value;
    }
    void set_interrupted(bool value) {
        is_interrupted_ = value;
    }
    bool is_out_of_memory() const { return is_out_of_memory_; }
    bool is_interrupted() const { return is_interrupted_; }
private:
    bool is_out_of_memory_;
    tbb::atomic<bool> is_interrupted_;
};
inline LuaUserData &getUserData(lua_State *L) {
    void *ud;
    lua_getallocf(L, &ud);
    return *static_cast<LuaUserData *>(ud);
}
}
#endif
