#ifndef HG_LUA_STACK_MANAGER_H
#define HG_LUA_STACK_MANAGER_H
#include "lua/lua.h"
namespace hg {
    class LuaStackManager final {
    public:
        explicit LuaStackManager(lua_State *L) : L(L), stack_height(lua_gettop(L)){}
        ~LuaStackManager() noexcept { lua_pop(L, lua_gettop(L) - stack_height); }
    private:
        lua_State *L;
        int stack_height;
    };
}
#endif //HG_LUA_STACK_MANAGER_H
