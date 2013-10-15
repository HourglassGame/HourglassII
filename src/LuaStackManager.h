#ifndef HG_LUA_STACK_MANAGER_H
#define HG_LUA_STACK_MANAGER_H
#include "lua/lua.h"
namespace hg {
    class LuaStackManager {
    public:
        explicit LuaStackManager(lua_State *L) : L_(L), stack_height_(lua_gettop(L_)){}
        ~LuaStackManager() { lua_pop(L_, lua_gettop(L_) - stack_height_); }
    private:
        lua_State *L_;
        int stack_height_;
    };
}
#endif //HG_LUA_STACK_MANAGER_H
