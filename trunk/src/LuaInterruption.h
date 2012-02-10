#ifndef HG_LUA_INTERRUPTION_H
#define HG_LUA_INTERRUPTION_H
#include "lua/lua.h"
#include "OperationInterrupter.h"
#include <boost/move/move.hpp>
namespace hg {
class LuaInterruptionHandle {
public:
    LuaInterruptionHandle() : L_(), functionHandle_(){}
    LuaInterruptionHandle(lua_State* L, OperationInterrupter::FunctionHandle functionHandle) :
        L_(L), functionHandle_(boost::move(functionHandle)) {}
    
    LuaInterruptionHandle(BOOST_RV_REF(LuaInterruptionHandle) o) :
        L_(o.L_), functionHandle_(boost::move(o.functionHandle_))
    {
        o.L_ = 0;
    }
    LuaInterruptionHandle& operator=(BOOST_RV_REF(LuaInterruptionHandle) o) {
        swap(o);
        return *this;
    }
    void swap(LuaInterruptionHandle& o) {
        boost::swap(L_, o.L_);
        boost::swap(functionHandle_, o.functionHandle_);
    }
    ~LuaInterruptionHandle() {
        if (L_) {
            lua_sethook(L_, 0, 0, 0);
        }
    }
private:
    lua_State* L_;
    OperationInterrupter::FunctionHandle functionHandle_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(LuaInterruptionHandle)
};
inline void swap(LuaInterruptionHandle& l, LuaInterruptionHandle& r) { l.swap(r); }

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
LuaInterruptionHandle makeInterruptable(lua_State* L, OperationInterrupter& interrupter);
}

#endif //HG_LUA_INTERRUPTION_H
