#include "LuaInterruption.h"
#include "LuaUserData.h"
namespace hg {
struct InterruptLua {
    InterruptLua(lua_State* L) :L_(L) {}
    void operator()() const {
        getUserData(L_).is_interrupted = true;
    }
private:
    lua_State* L_;
};

static void interruption_hook(lua_State* L, lua_Debug*)
{
    if (getUserData(L).is_interrupted) {
        lua_pushnil(L);
        lua_error(L);
    }
}

//OperationInterrupter::FunctionHandle
LuaInterruptionHandle makeInterruptable(lua_State* L, OperationInterrupter& interrupter)
{
    lua_sethook(L, interruption_hook, LUA_MASKCOUNT, 1024);
    return LuaInterruptionHandle(L, interrupter.addInterruptionFunction(move_function<void()>(InterruptLua(L))));
}
}
