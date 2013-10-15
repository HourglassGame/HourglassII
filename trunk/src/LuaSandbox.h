#ifndef HG_LUA_SANDBOX_H
#define HG_LUA_SANDBOX_H
struct lua_State;
namespace hg {
    void loadSandboxedLibraries(lua_State *L);
    void setPackagePreloadResetFunction(lua_State *L);

    //Restores the table at RIDX_GLOBALS to the safe values loaded by `loadSandboxedLibraries`
    //(thus sandboxing any functions that have this table as their _ENV upvalue)
    void restoreGlobals(lua_State *L);
}

#endif //HG_LUA_SANDBOX_H
