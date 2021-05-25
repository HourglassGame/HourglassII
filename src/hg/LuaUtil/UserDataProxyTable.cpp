#include "UserDataProxyTable.h"
#include "lua/lauxlib.h"
#include <memory>
#include <cassert>
namespace hg {

namespace {
    char const metatableRegistryKey[] = "HG_UDPT_MetaTable";
    char const nilProxyRegistryKey[] = "HG_UDPT_Nil";
    
    int UDPT_newindex(lua_State *L) {
        //Args: UDPT, key, value
        //(No need to check the arguments, because they cannot ever
        // be anything else (because metamethods
        // can never be invoked from user code if properly
        // protected).
        
        //if (value != nil) {
        //    UDPT.outer[key] = value;
        //}
        //else {
        //    UDPT.outer[key] = HG_UDPT_Nil;
        //}
        
        UDPT *udpt(static_cast<UDPT *>(lua_touserdata(L, 1)));
        assert(udpt);
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, udpt->outerTable);
        
        if (!lua_isnil(L, 3)) {
            //UDPT.outer[key] = value
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_rawset(L, 4);
        }
        else {
            //UDPT.outer[key] = HG_UDPT_Nil
            lua_pushvalue(L, 2);
            luaL_getmetatable(L, nilProxyRegistryKey);
            lua_rawset(L, 4);
        }
        lua_pop(L, 4);
        return 0;
    }
    
    int UDPT_index(lua_State *L) {
        //Args: UDPT, key
        
        //v = UDPT.outer[key]
        //if (v != nil) {
        //    if (v == HG_UDPT_Nil) {
        //        return nil
        //    }
        //    else {
        //        return v
        //    }
        //}
        //else {
        //    return UDPT.inner[key]
        //}
        
        //UDPT, key
        UDPT *udpt(static_cast<UDPT *>(lua_touserdata(L, 1)));
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, udpt->outerTable);
        //UDPT, key, outerTable
        lua_pushvalue(L, 2);
        //UDPT, key, outertable, key
        lua_rawget(L, -2);
        //UDPT, key, outertable, outertable[key]
        if (!lua_isnil(L, -1)) {
            luaL_getmetatable(L, nilProxyRegistryKey);
            //UDPT, key, outertable, outertable[key], HG_UDPT_nil
            if (lua_rawequal(L, -1, -2)) {
                lua_pushnil(L);
                //UDPT, key, outertable, outertable[key], HG_UDPT_nil, nil
            }
            else {
                lua_pop(L, 1);
                //UDPT, key, outertable, outertable[key]
            }
        }
        else {
            lua_pop(L, 2);
            //UDPT, key
            lua_rawgeti(L, LUA_REGISTRYINDEX, udpt->baseTable);
            //UDPT, key, innerTable
            lua_pushvalue(L, 2);
            //UDPT, key, innertable, key
            lua_rawget(L, -2);
            //UDPT, key, innertable, innertable[key]
        }
        return 1;
    }
    
    int UDPT_gc(lua_State *L) {
        //Args: UDPT
        UDPT *udpt(static_cast<UDPT *>(lua_touserdata(L, 1)));
        assert(udpt);
        luaL_unref(L, LUA_REGISTRYINDEX, udpt->baseTable);
        luaL_unref(L, LUA_REGISTRYINDEX, udpt->outerTable);
        lua_pop(L, 1);
        return 0;
    }
}
//UDPT library initialization.
//Must be called exactly once (per
//lua_State) prior to the use of any other
//UDPT functions.

//Loads a set of "magic" variables
//into the registry of the given
//lua_State. These include the
//metatable used by UDPTs, as well as
//the magic constant used to represent nil.
void load_UDPT_lib(lua_State *L) {
    int nilProxyCreated(luaL_newmetatable(L, nilProxyRegistryKey));
    assert(nilProxyCreated); (void)nilProxyCreated;
    lua_pop(L, 1);
    int metatableCreated(luaL_newmetatable(L, metatableRegistryKey));
    assert(metatableCreated); (void)metatableCreated;
    
    //__len
    //not yet implemented
    //__metatable
    lua_newtable(L);
    lua_setfield(L, -2, "__metatable");
    //__index
    lua_pushcfunction(L, UDPT_index);
    lua_setfield(L, -2, "__index");
    //__newindex
    lua_pushcfunction(L, UDPT_newindex);
    lua_setfield(L, -2, "__newindex");
    //__gc
    lua_pushcfunction(L, UDPT_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}


struct LuaRef {
    operator void const*() const {
        return val == LUA_NOREF ? nullptr : &val;
    }
    
    LuaRef(LuaRef const &) = default;
    LuaRef(LuaRef &&) = default;
    LuaRef &operator=(LuaRef const &) = default;
    LuaRef &operator=(LuaRef &&) = default;

    LuaRef &operator=(std::nullptr_t) {
        val = LUA_NOREF;
        return *this;
    }

    LuaRef(int val = LUA_NOREF) : val(val) {}

    int val;
};

struct LuaRefDeleter {
    explicit LuaRefDeleter(lua_State *L) : L(L) {}
    void operator()(LuaRef ref) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref.val);
    }
    typedef LuaRef pointer;
private:
    lua_State *L;
};

//[-1,+1,m]
//Pops the object at the top of the
//stack (which must be a table), and
//pushes a proxy table which wraps
//that table.
int create_proxy_table(lua_State *L) {
    //Create outer table
    lua_newtable(L);
    
    //Create refs to the base table and the outer table
    std::unique_ptr<LuaRef, LuaRefDeleter> outerRef(LuaRef{luaL_ref(L, LUA_REGISTRYINDEX)}, LuaRefDeleter(L));
    std::unique_ptr<LuaRef, LuaRefDeleter>  baseRef(LuaRef{luaL_ref(L, LUA_REGISTRYINDEX)}, LuaRefDeleter(L));

    //Create the proxy table -- give it sufficient size to carry two "references"
    UDPT *rawProxyTable(static_cast<UDPT *>(lua_newuserdata(L, sizeof (UDPT))));
    
    //Set its fields appropriately (with the base table and outer table).
    //None of these operations should throw.
    luaL_setmetatable(L, metatableRegistryKey);
    rawProxyTable->baseTable =  baseRef.release().val;
    rawProxyTable->outerTable = outerRef.release().val;
    
    //Return
    return 1;
}
//[0,+1,-]
void get_base_table(lua_State *L, UDPT const *proxyTable) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, proxyTable->baseTable);
}
//[-1,0,e]
void set_outer_table(lua_State *L, UDPT const *proxyTable) {
    lua_rawseti(L, LUA_REGISTRYINDEX, proxyTable->outerTable);
}

int UDPT_wrap_all(lua_State * const L) {
    (void)L;
    return 1;
}

}
