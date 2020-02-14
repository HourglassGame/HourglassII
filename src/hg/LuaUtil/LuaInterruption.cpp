#include "LuaInterruption.h"
#include "LuaUserData.h"
#include <thread>
namespace hg {
static void interruption_hook(lua_State *L, lua_Debug *)
{
    if (getUserData(L).is_interrupted()) {
        lua_pushnil(L);
        lua_error(L);
    }
}

LuaInterruptionHandle makeInterruptable(lua_State *L, OperationInterrupter &interrupter)
{
    auto &userData = getUserData(L);
    lua_sethook(L, interruption_hook, LUA_MASKCOUNT, 1024);
    return LuaInterruptionHandle(
            L,
            interrupter.addInterruptionFunction([&]{userData.set_interrupted(true);}));
}
}
