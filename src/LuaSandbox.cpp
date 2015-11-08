#include "LuaSandbox.h"

#include "UserDataProxyTable.h"

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/range/algorithm/find_if.hpp>

//#include "lua/lstate.h"

namespace hg {

//Performs the equivalent of t.name = v, where t is the table
//at the given valid index and v is the value at the top of the
//stack.

//Unlike in Lua, this operation will create any subtables that are encountered
//during the traversal if they do not yet exist.

//Example: if t = {}, v = 10 then
//recursive_setfield(L, indexof(T), "b.c") is equivalent to t.b={c=10}

//As in lua, this may trigger metamethods on tables that exist.
static void recursive_setfield(lua_State *L, int index, char const *name) {
    //[..., t, ..., v]
    using boost::algorithm::split_iterator;
    using boost::algorithm::first_finder;
    bool firstTime(true);
    int const absidx(lua_absindex(L, index));
    int const v_index(lua_gettop(L));
    split_iterator<char const*> current(name, name + std::strlen(name), first_finder("."));
    split_iterator<char const*> next(boost::next(current));
    for (; !next.eof(); current = next, ++next) {
        //This demands that the subtable be an *actual* table
        //(not a proxy table). This may not be ideal, so
        //fix it if it becomes a problem.
        luaL_getsubtable(L, firstTime ? absidx : -2,
            std::string(boost::begin(*current), boost::end(*current)).c_str()); //[..., t, ..., v, x?, y]
        if (!firstTime) lua_replace(L, -2); //[..., t, ..., v, y]
        firstTime = false;
    }
    lua_pushvalue(L, v_index);//[..., t, ..., v, y?, v]
    lua_setfield(L, firstTime ? absidx : -2, std::string(boost::begin(*current), boost::end(*current)).c_str());//[..., t, ..., v, y?]
    lua_pop(L, firstTime ? 1 : 2);//[..., t, ...]
}

//Pushes onto the stack the value t.name, where t is the value at the given valid index.
//`name` must be a valid field name or sequence of field names.

//Example: if t = {b = {c = 10}} then
//recursive_getfield(L, indexof(T), "b.c") pushes `10` on to the stack.

//As in lua, this may trigger metamethods, and attempts to index into non-tables will cause
//an error to be raised (not yet implemented).
static void recursive_getfield(lua_State *L, int index, char const *name) {
    //[..., t, ...]
    using boost::algorithm::split_iterator;
    using boost::algorithm::first_finder;
    bool firstTime(true);
    int absidx(lua_absindex(L, index));
    for (split_iterator<char const*> it(name, name + std::strlen(name), first_finder(".")); !it.eof(); ++it) {
        lua_pushlstring(L, boost::begin(*it), boost::distance(*it));//[..., t, ..., x?, str]
        lua_gettable(L, firstTime ? absidx : -2); //[..., t, ..., x?, v]
        if (!firstTime) lua_replace(L, -2); //[..., t, ..., v]
        firstTime = false;
    }
}

static char const *safe_elements[] = {
"assert",
"error",
"ipairs",
"next",
"pairs",
//"pcall", //pcall and xpcall could catch the interruption error
           //(see makeInterruptable). We need a custom version of them.
"print",   //Unsafe, but useful for debugging
"require", //Only safe when other conditions are met:
           //-- Its upvalue refers to the appropriate package table
           //-- package.searchers only contains safe searchers
           //   (eg, only the preload searcher)
           //-- package.loaded gets cleaned between runs, and the
           //   packages tables that it contains are proxied
"select",
"tonumber",
"tostring",
"type",
"unpack",
"_VERSION",
//"xpcall", //see pcall
"coroutine.create",
"coroutine.resume",
"coroutine.running",
"coroutine.status",
"coroutine.wrap",
"coroutine.yield",//TODO - properly investigate the security implications of allowing this
//"package.loaded", //These three must be actual tables (not proxy tables) (because they are
//"package.preload",//used internally by `require`). To make this safe, they must be manually
//"package.searchers",//restored to safe values before each run. See `restoreGlobals` for details.
"string.byte",
"string.char",
"string.find",
"string.format",
"string.gmatch",
"string.gsub",
"string.len",
"string.lower",
"string.match",
"string.rep",
"string.reverse",
"string.sub",
"string.upper",
"table.insert",
"table.maxn",
"table.remove",
"table.sort",
"math.abs",
"math.acos",
"math.asin",
"math.atan",
"math.atan2",
"math.ceil",
"math.cos",
"math.cosh",
"math.deg",
"math.exp",
"math.floor",
"math.fmod",
"math.frexp",
"math.huge",
"math.ldexp",
"math.log",
"math.log10",
"math.max",
"math.min",
"math.modf",
"math.pi",
"math.pow",
"math.rad",
"math.sin",
"math.sinh",
"math.sqrt",
"math.tan",
"math.tanh"
};

static char const *tables_to_wrap[] = {
"bit32",
"coroutine",
"package",
"string",
"table",
"math"
};

char const preloadResetRegistryIndex[] = "preloadReset";
char const loadedResetRegistryIndex[] = "loadedReset";
char const packageSearchersResetRegistryIndex[] = "packageSearchersReset";
static int packageSearchersReset(lua_State *L) {
    //return {upvalues[1]}  -- upvalues[1] should contain the preload searcher
    
    //[]
    lua_createtable(L, 1, 0);//[table]
    lua_pushvalue(L, lua_upvalueindex(1)); //[table, preload searcher]
    lua_rawseti(L, -2, 1);//[table]
    
    return 1;
}

static int loadedReset(lua_State *L) {
    //return deep_copy(upvalues[1])  -- upvalues[1] should contain the prototype _LOADED table
    
    lua_newtable(L);//[newLoaded]
    lua_pushvalue(L, lua_upvalueindex(1)); //[newLoaded, proto]
    
    lua_pushnil(L);//[newLoaded, proto, nil]
    while (lua_next(L, -2) != 0) {//[newLoaded, proto, key, value]
        lua_pushvalue(L, -2);//[newLoaded, proto, key, value, key]
        lua_insert(L, -2);//[newLoaded, proto, key, key, value]
        lua_settable(L, -5);//[newLoaded, proto, key]
    }
    //[newLoaded, proto]
    lua_pop(L, 1);//[newLoaded]
    return 1;
}

static int defaultPreloadReset(lua_State *L) {
    //return {}
    lua_newtable(L);
    return 1;
}

static void sandboxGlobalTable(lua_State *L) {
    //[original_globals]
    lua_newtable(L); //[original_globals, new_globals]
    
    for (char const *name: safe_elements) {
    //  new_globals.name = original_globals.name
        recursive_getfield(L, -2, name); //[original_globals, new_globals, v]
        recursive_setfield(L, -2, name); //[original_globals, new_globals]
    }
    
    //Manually insert an empty base table for `package`
    lua_newtable(L);//[original_globals, new_globals, package]
    lua_setfield(L, -2, "package"); //[original_globals, new_globals]

    lua_remove(L, -2); //[new_globals]
    //At this stage, the upvalues in the `package` functions are referring to the
    //wrong table. (this will be fixed at a later stage in `touchupPackage`)
    //(The relevant functions are `require` and `package.searchers[1]` (the preload searcher)).
}

static void touchupPackage(lua_State *L) {
    //[preload searcher]
    int const preloadidx(lua_absindex(L, -1));
    
    //g = registry.globals.base
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);//[preload searcher,globals]
    UDPT *globals(static_cast<UDPT *>(lua_touserdata(L, -1)));//[preload searcher,globals]
    lua_pop(L, 1);//[preload searcher]
    get_base_table(L, globals);//[preload searcher,g]
    int const gidx(lua_absindex(L, -1));
    
    //preload searcher.upvalues[1] = g.package
    lua_getfield(L, gidx, "package");//[preload searcher,g,package]
    lua_setupvalue(L, preloadidx, 1);//[preload searcher,g]
    
    //g.require.upvalues[1] = g.package
    lua_getfield(L, gidx, "require");//[preload searcher,g,require]
    lua_getfield(L, gidx, "package");//[preload searcher,g,require,package]
    lua_setupvalue(L, -2, 1);//[preload searcher,g,require]
    lua_pop(L, 1);//[preload searcher,g]
    
    //loadedPrototype = {}
    lua_newtable(L);//[preload searcher,g,loadedPrototype]
    
    for (char const *name: tables_to_wrap) {
        //loadedPrototype.name = g.name
        lua_getfield(L, gidx, name);//[preload searcher,g,loadedPrototype,g.name]
        lua_setfield(L, -2, name);//[preload searcher,g,loadedPrototype]
    }
    
    //registry.preloadReset = defaultPreloadReset
    lua_pushcfunction(L, defaultPreloadReset);//[preload searcher,g,loadedPrototype,defaultPreloadReset]
    lua_setfield(L, LUA_REGISTRYINDEX, preloadResetRegistryIndex);//[preload searcher,g,loadedPrototype]
    
    //registry.loadedReset = loadedResetFunction (with upvalue[1] = loadedPrototype)
    lua_pushcclosure(L, loadedReset, 1);//[preload searcher,g,loadedResetFunction]
    lua_setfield(L, LUA_REGISTRYINDEX, loadedResetRegistryIndex);//[preload searcher,g]
    
    lua_pop(L, 1);//[preload searcher]
    //registry.packageSearchersReset = packageSearchersResetFunction (with upvalue[1] = preload searcher)
    lua_pushcclosure(L, packageSearchersReset, 1);//[packageSearchersResetFunction]
    lua_setfield(L, LUA_REGISTRYINDEX, packageSearchersResetRegistryIndex);//[]
}

static void createProxyGlobals(lua_State *L) {
    //[...]
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS); //[..., globals]

    for (char const *name: tables_to_wrap) {
        lua_getfield(L, -1, name); //[..., globals, g.name]
        create_proxy_table(L);     //[..., globals, t]
        lua_setfield(L, -2, name); //[..., globals]
    }
    
    create_proxy_table(L);//[...,globalproxy]
    lua_rawseti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);//[...]
}


//This sandboxing (when correctly applied) stops the following occurences:
// - (persistent) modification to (both Lua and C++) globals 
// - access to the environment (files, time, etc)
// - undefined behaviour

//Other stuff is also blocked as convenient. This is a whitelist of required and useful functions that
//are know to be safe.

//This sandboxing is a two step process.
//Usage:
//  Create a lua_State
//  call loadSandboxedLibraries (this loads the stuff required for sandboxFunction to work
//                               (including many safe library functions))
//  DO NOT ATTEMPT TO ADD ANY NEW LIBRARIES, as loadSandboxedLibraries messes with
//  Lua's internal library loading bookkeeping datastructures.
//  Add additional libraries with setPackagePreloadResetFunction if desired (in contrast
//  with the warning to not load any additional libraries, loaders set up in this way will work)

//Now -- each time that you want to run a chunk in a sandbox, follow the following procedure:
//  load the chunk
//  call sandboxFunction(lua_State *L, int index), which sets the _ENV upvalue
//        of the given function to contain a global environment that is totally
//        independent from all other environments.
//  run the chunk

//This two stage process is justified for performance reasons.
//`loadSandboxedLibraries` is an expensive operation, but it
//does work that should make `sandboxFunction` faster.
void loadSandboxedLibraries(lua_State *L)
{
    //[...]
    //Load all libs
    //the io, debug and os libs are not used, so they should probably not be loaded.
    luaL_openlibs(L);//[...]
    load_UDPT_lib(L);//[...]
    //UDPT wrap stuff that happens to look like standard global functions
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);//[rawglobals]
    UDPT_wrap_all(L);//[...,rawglobals]
    //save the preload searcher for later
    recursive_getfield(L, -1, "package.searchers");//[...,rawglobals, pkg.searchers]
    lua_rawgeti(L, -1, 1);//[...,rawglobals, pkg.searchers, preload searcher]
    lua_insert(L, -3);     //[...,preload searcher, rawglobals, pkg.searchers]
    lua_pop(L, 1);        //[...,preload searcher, rawglobals]
    //Modify the global table (and sub-tables) to only hold the elements
    //that are safe.
    sandboxGlobalTable(L);//[...,preload searcher, safeglobals]
    lua_rawseti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);//[...,preload searcher]
    //Replace the global table and the package tables
    //with proxy tables.
    createProxyGlobals(L);//[...,preload searcher]
    //Modify functions in the modules package to
    //refer to the proxied `package` table (with their upvalues).
    //(this means: `require` and the preload searcher)
    //Modify _LOADED to contain just the proxied package tables for the safe packages.
    //Save a copy of the table in _LOADED for later restoration by
    //registry.loadedReset.
    //Set up loadedReset, preloadReset and a default packageSearchersReset
    touchupPackage(L);//[]
}
//[0, -1, m]
void setPackagePreloadResetFunction(lua_State *L) {
    //registry.preloadReset = pop(stack)
    lua_setfield(L, LUA_REGISTRYINDEX, preloadResetRegistryIndex);
}

void restoreGlobals(lua_State *L) {
    assert(lua_status(L) == LUA_OK);
    //registry.globals.outer = {}
    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);//[globals]
    UDPT *globals(static_cast<UDPT *>(lua_touserdata(L, -1)));//[globals]
    assert(globals);
    lua_pop(L, 1);//[]
    lua_newtable(L);//[{}]
    set_outer_table(L, globals);//[]
    get_base_table(L, globals);//[g]
    
    for (char const *name: tables_to_wrap) {//[g]
        //g.name.outer = {}
        lua_getfield(L, -1, name);//[g,g.name]
        UDPT *package_table(static_cast<UDPT*>(lua_touserdata(L, -1)));//[g,g.name]
        assert(package_table);
        lua_pop(L, 1);//[g]
        lua_newtable(L);//[g, {}]
        set_outer_table(L, package_table);//[g]
    }
    lua_getfield(L, -1, "package");//[g,package]
    UDPT *package(static_cast<UDPT*>(lua_touserdata(L, -1)));
    assert(package);
    lua_pop(L, 2);//[]
    get_base_table(L, package);//[packagebase]
    
    //preload = registry.preloadReset()
    lua_getfield(L, LUA_REGISTRYINDEX, preloadResetRegistryIndex);//[packagebase, preloadReset]
    lua_call(L, 0, 1);//[packagebase, preload]
    lua_pushvalue(L, -1);//[packagebase, preload, preload]
    //registry.preload = preload
    lua_setfield(L, LUA_REGISTRYINDEX, "_PRELOAD");//[packagebase, preload]
    //packagebase.preload = preload
    lua_setfield(L, -2, "preload");//[packagebase]
    
    //loaded = registry.loadedReset()
    lua_getfield(L, LUA_REGISTRYINDEX, loadedResetRegistryIndex);//[packagebase, loadedReset]
    lua_call(L, 0, 1);//[packagebase, loaded]
    lua_pushvalue(L, -1);//[packagebase, loaded, loaded]
    //registry.loaded = loaded
    lua_setfield(L, LUA_REGISTRYINDEX, "_LOADED");//[packagebase, loaded]
    //packagebase.loaded = loaded
    lua_setfield(L, -2, "loaded");//[packagebase]
    
    //packagebase.searchers = registry.packageSearchersReset()
    lua_getfield(L, LUA_REGISTRYINDEX, packageSearchersResetRegistryIndex);//[packagebase, packageSearchersReset]
    lua_call(L, 0, 1);//[packagebase, searchers]
    lua_setfield(L, -2, "searchers");//[packagebase]
    
    lua_pop(L, 1);//[]
}
}//namespace hg
