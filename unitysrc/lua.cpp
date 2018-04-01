#include "../src/Lua_test.cpp"

//luaconf.h changes behaviour depending on whether or not these are defined.
//Make sure they are all defined up front, so the first time that luaconf.h is included,
//all relevant macros/functions/whatever are made available (because from the second time onwards,
//luaconf.h is protected by an include guard).
#define LUA_LIB
#define LUA_CORE

#include "../src/lua/luaconf.h"

#include "../src/lua/lapi.cpp"
#include "../src/lua/lauxlib.cpp"
#include "../src/lua/lbaselib.cpp"
#include "../src/lua/lbitlib.cpp"
#include "../src/lua/lcode.cpp"
#include "../src/lua/lcorolib.cpp"
#include "../src/lua/lctype.cpp"
#include "../src/lua/ldblib.cpp"
#include "../src/lua/ldebug.cpp"
#include "../src/lua/ldo.cpp"
#include "../src/lua/ldump.cpp"
#include "../src/lua/lfunc.cpp"
#include "../src/lua/lgc.cpp"
#include "../src/lua/linit.cpp"
#include "../src/lua/liolib.cpp"
#include "../src/lua/llex.cpp"
#include "../src/lua/lmathlib.cpp"
#include "../src/lua/lmem.cpp"
#include "../src/lua/loadlib.cpp"
#include "../src/lua/lobject.cpp"
#include "../src/lua/lopcodes.cpp"
#include "../src/lua/loslib.cpp"
#include "../src/lua/lparser.cpp"
#include "../src/lua/lstate.cpp"
#include "../src/lua/lstring.cpp"
#include "../src/lua/lstrlib.cpp"
#include "../src/lua/ltable.cpp"
#include "../src/lua/ltablib.cpp"
#include "../src/lua/ltm.cpp"
#include "../src/lua/lundump.cpp"
#include "../src/lua/lutf8lib.cpp"
#include "../src/lua/lvm.cpp"
#include "../src/lua/lzio.cpp"


#undef LUA_LIB
#undef LUA_CORE
