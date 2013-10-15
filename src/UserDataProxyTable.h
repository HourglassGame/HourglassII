#ifndef HG_USER_DATA_PROXY_TABLE_H
#define HG_USER_DATA_PROXY_TABLE_H
#include "lua/lua.h"
//User Data Proxy Table:
//A UserData that wraps a "base" table, and
//acts like a (shallow) copy of the base table (except that
//the copy operation is fast (at the expense of reads
//and writes being slightly slower)).
//In HourglassII, this is used as an optimisation
//for part of the lua sandboxing. (e.g. Executions
//of the trigger system should logically occur in
//total isolation from each other, but creating a
//lua_State and initialising its libraries every
//execute-frame is expensive. It is cheaper to
//have a single lua_State, and to give
//each frame a unique copy of the global table. This
//would be expensive were copies expensive, so a UDPT is used
//as an optimization.
//(See DirectLuaTriggerSystem)

//The current implementation is somewhat less than complete (especially the sandboxing part).
//In particular, the commented out functions are not wrapped, even though they probably should
//be.
namespace hg {
    struct UDPT {
        int baseTable;
        int outerTable;
    };

//Implementation:
//Holds two tables -- the `base` table and the `outer` table.
//Reads are first searched for in the outer table, and fall through to the
//base table if the element does not exist in the outer table
//Writes act only on the outer table. If a key gets associated with
//nil, a special placeholder (lightuserdata) is inserted instead, and this gets read as nil.
    void load_UDPT_lib(lua_State *L);

    //Create proxy table
    int create_proxy_table(lua_State *L);
    
    void get_base_table(lua_State *L, UDPT const *proxyTable);
    void set_outer_table(lua_State *L, UDPT const *proxyTable);
    
    //Wrap built-in functions to make the UDPT act like a table
    //These are all lua_Cfunction compatible functions.
    //From a Lua perspective they take a function and
    //return a wrapped version of that function that makes the function
    //act correctly when given a UDPT.
    //int wrap_getmetatable(lua_State *L);//metatable use the __metatable metafield
    //int wrap_ipairs(lua_State *L);//will use the __ipairs metamethod
    //int wrap_next(lua_State *L);//should be wrapped, but unimportant
    //int wrap_pairs(lua_State *L);//will use the __pairs metamethod
    //  raw* functions will be sandboxed
    //int wrap_rawequal(lua_State *L);
    //int wrap_rawget(lua_State *L);
    //int wrap_rawlen(lua_State *L);
    //int wrap_rawset(lua_State *L);
    //int wrap_setmetatable(lua_State *L);//metatable accesses will be sandboxed
    
    //int wrap_tostring(lua_State *L);//not yet implemented
    //int wrap_type(lua_State *L);//not yet implemented
    
    //debug_* will be sandboxed
    //wrap_debug_*
    
    //   Should be implemented, but unimportant at the moment
    //int wrap_table_concat(lua_State *L);
    //int wrap_table_insert(lua_State *L);
    //int wrap_table_pack(lua_State *L);
    //int wrap_table_remove(lua_State *L);
    //int wrap_table_sort(lua_State *L);
    //int wrap_table_unpack(lua_State *L);
    
    
    //Convenience wrapper for all the other wrap_* functions.
    //Takes a table and looks for the functions that need to be wrapped in their normal
    //locations (if the given table were the global function table)
    //and modifies that table to contain the wrapped versions. Returns the modified table.
    int UDPT_wrap_all(lua_State *L);//Nothing to do... entirely (but incompletely) implemented with metatables (for now)...
}
#endif //HG_USER_DATA_PROXY_TABLE_H

