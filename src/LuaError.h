#ifndef HG_LUA_ERROR_H
#define HG_LUA_ERROR_H
#include "lua/lua.h"
#include <string>
#include <vector>
#include <exception>
#include "prettyprint.hpp"
#include <boost/exception/all.hpp>
#include <iostream>
namespace hg {
    struct semantic_callstack final {};
    typedef boost::error_info<semantic_callstack, std::vector<std::string>> semantic_callstack_info;

    struct basic_error_message final {};
    typedef boost::error_info<basic_error_message, std::string> basic_error_message_info;

    struct LuaError : virtual boost::exception, virtual std::exception
    {
        virtual ~LuaError() noexcept override = 0;
    };
    inline LuaError::~LuaError() noexcept
    {
    }

    //A panic/fatal error occurred within the execution of some lua.
    struct LuaRunError : LuaError {
        explicit LuaRunError(lua_State *L) {
            int n{0};
            while (true) {
                lua_Debug ar;
                if (lua_getstack(L, n, &ar) == 0) break;
                int rVal = lua_getinfo(L, "nSl", &ar);
                assert(rVal != 0); //TODO: Handle rVal != 0 properly?
                std::cout << "Frame: " << n << "\n" << std::flush;
                std::cout << "  name: " << (ar.name ? ar.name : "^__NULL__") << "\n" << std::flush;
                std::cout << "  linedefined: " << ar.linedefined << "\n" << std::flush;
                std::cout << "  currentline: " << ar.currentline << "\n" << std::flush;
                std::cout << "\n" << std::flush;
                ++n;
            }

            *this << basic_error_message_info(std::string(lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : ""));
            //TODO- Extract and store more complete debug data as appropriate
            //lua_Debug ar;
            //lua_getStack in a loop or something...
        }
        virtual ~LuaRunError() noexcept override {}
    };

    //Some lua returned values that did not meet the type constraints of the interface that the Lua
    //was meant to be implementing.
    struct LuaInterfaceError : LuaError {
        explicit LuaInterfaceError()
        {
        }
        virtual ~LuaInterfaceError() noexcept override {}
    };

    inline void add_semantic_callstack_info(LuaError &e, std::string info) {
        std::vector<std::string> *callstackInfo = boost::get_error_info<semantic_callstack_info>(e);
        if (callstackInfo) {
            callstackInfo->push_back(std::move(info));
        }
        else {
            e << semantic_callstack_info(std::vector<std::string>{std::move(info)});
        }
    }
}
#endif //HG_LUA_ERROR_H
