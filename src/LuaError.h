#ifndef HG_LUA_ERROR_H
#define HG_LUA_ERROR_H
#include "lua/lua.h"
#include <string>
namespace hg {
    struct LuaError {
        LuaError(lua_State* L) : message(lua_type(L, -1) == LUA_TSTRING?lua_tostring(L, -1):"") {
            //TODO- Extract and store more complete debug data as appropriate
        }
        std::string message;
    };
}
#endif //HG_LUA_ERROR_H
