#ifndef HG_LUA_INTERRUPTION_H
#define HG_LUA_INTERRUPTION_H
#include "lua/lua.h"
#include "OperationInterrupter.h"
#include <utility>
namespace hg {
class LuaInterruptionHandle {
public:
    LuaInterruptionHandle() = default;
    LuaInterruptionHandle(lua_State *L, OperationInterrupter::FunctionHandle functionHandle) :
        L(L), functionHandle(std::move(functionHandle)) {}
    
    LuaInterruptionHandle(LuaInterruptionHandle &&o) noexcept :
        L(o.L), functionHandle(std::move(o.functionHandle))
    {
        o.L = nullptr;
    }
    LuaInterruptionHandle &operator=(LuaInterruptionHandle &&o) noexcept {
        swap(o);
        return *this;
    }
    void swap(LuaInterruptionHandle &o) noexcept {
        boost::swap(L, o.L);
        boost::swap(functionHandle, o.functionHandle);
    }
    ~LuaInterruptionHandle() noexcept {
        if (L) {
            lua_sethook(L, nullptr, 0, 0);
        }
    }
private:
    lua_State *L;
    OperationInterrupter::FunctionHandle functionHandle;
};
inline void swap(LuaInterruptionHandle &l, LuaInterruptionHandle &r) noexcept { l.swap(r); }

//Registers a interruptionFunction with interruper.
//Also puts a debug hook into L, which checks an interruption flag and raises an error if it is set.
//The interruptionFunction which is registered sets this interruption flag.
//Returns a LuaInterruptionHandle, which deregisters the debug hook and the interruption function
//when its lifetime ends.

//Implications:
//1. The allocator userdata must have type LuaUserData*.
//2. Any debug hooks already present will be replaced.
//3. Setting a new debug hook will inhibit the ability of the lua_State
//   to be interrupted.
//4. The interruption error must not be caught by the lua code, and when it is
//   eventually caught, it must result in an interrupted_exception.
LuaInterruptionHandle makeInterruptable(lua_State *L, OperationInterrupter &interrupter);
}

#endif //HG_LUA_INTERRUPTION_H
