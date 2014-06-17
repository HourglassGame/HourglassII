#ifndef HG_LUA_ERROR_H
#define HG_LUA_ERROR_H
#include "lua/lua.h"
#include <string>
#include <exception>
namespace hg {
    struct LuaError : std::exception {
        explicit LuaError(lua_State *L) : message(lua_type(L, -1) == LUA_TSTRING?lua_tostring(L, -1):"") {
            //TODO- Extract and store more complete debug data as appropriate
        }
        std::string message;
        char const *what() const noexcept override {
            return message.c_str();
        }
    };
}
#endif //HG_LUA_ERROR_H
