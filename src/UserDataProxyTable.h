#ifndef HG_USER_DATA_PROXY_TABLE_H
#define HG_USER_DATA_PROXY_TABLE_H
//User Data Proxy Table:
//A UserData that wraps two tables, a 
//Initial use is to optimise a table that must be copied
//very often (for sandboxing), but whose copies are actually
//changed very rarely.
namespace hg {
    //Create proxy table
    
    //Wrap built-in functions to make the UDPT act like a table
    //These are all lua_Cfunction compatible functions.
    //From a Lua perspective they take a function and
    //return a wrapped version of that function that makes the function
    //act correctly when given a UDPT.
    void wrap_istable(lua_State* L);
    
    //Convenience wrapper for all the other wrap_* functions.
    //Takes a table and looks for the functions that need to be wrapped in their normal
    //locations (if the given table were the global function table
    //This does not work with UDBT
    void wrap_all(lua_State* L);
}
#endif //HG_USER_DATA_PROXY_TABLE_H

