//luaconf.h changes behaviour depending on whether or not these are defined.
//Make sure they are all defined up front, so the first time that luaconf.h is included,
//all relevant macros/functions/whatever are made available (because from the second time onwards,
//luaconf.h is protected by an include guard).
#define LUA_LIB
#define LUA_CORE

#include "lua/luaconf.h"

#include "lua/lapi.cpp"
#include "lua/lauxlib.cpp"
#include "lua/lbaselib.cpp"
#include "lua/lcode.cpp"
#include "lua/lcorolib.cpp"
#include "lua/lctype.cpp"
#include "lua/ldblib.cpp"
#include "lua/ldebug.cpp"
#include "lua/ldo.cpp"
#include "lua/ldump.cpp"
#include "lua/lfunc.cpp"
#include "lua/lgc.cpp"
#include "lua/linit.cpp"
#include "lua/liolib.cpp"
#include "lua/llex.cpp"
#include "lua/lmathlib.cpp"
#include "lua/lmem.cpp"
#include "lua/loadlib.cpp"
#include "lua/lobject.cpp"
#include "lua/lopcodes.cpp"
#include "lua/loslib.cpp"
#include "lua/lparser.cpp"
#include "lua/lstate.cpp"
#include "lua/lstring.cpp"
#include "lua/lstrlib.cpp"
#include "lua/ltable.cpp"
#include "lua/ltablib.cpp"
#include "lua/ltm.cpp"
#include "lua/lundump.cpp"
#include "lua/lutf8lib.cpp"
#include "lua/lvm.cpp"
#include "lua/lzio.cpp"

#undef LUA_LIB
#undef LUA_CORE
