#ifndef HG_LUA_UTILITIES_H
#define HG_LUA_UTILITIES_H
#include "SimpleLuaCpp.h"
#include "Environment.h"
#include <vector>
#include <string>
#include "InitialGuyArrival.h"
#include "InitialObjects.h"
#include "InitialBox.h"
#include "InitialGuy.h"
#include "TriggerOffsetsAndDefaults.h"
#include "TriggerSystem.h"
#include "DirectLuaTriggerSystem.h"
#include "unique_ptr.h"
#include "OperationInterruptor.h"

//Miscelaneous things related to lua that are useful in multiple places
//Should be replaced by a more extensible and cohesive model eventually
namespace hg {
const char * lua_VectorReader (
    lua_State *L,
    void *ud,
    size_t *size);
int lua_VectorWriter(
    lua_State *L,
    const void* p,
    size_t sz,
    void* ud);
std::vector<char> loadFileIntoVector(
    std::string const& filename,
    OperationInterruptor const& interruptor = g_nullInterruptor);
LuaState loadLuaStateFromVector(std::vector<char> const& luaData, std::string const& chunkName);
template<typename T>
T to(lua_State* L, int index = -1);
template<typename T>
T readField(lua_State* L, char const* fieldName, int index = -1)
{
    lua_getfield(L, index, fieldName);
    T retv(to<T>(L));
    lua_pop(L, 1);
    return retv;
}
template<typename T>
T readGlobal(lua_State* L, char const* globalName)
{
    return readField<T>(L, globalName, LUA_GLOBALSINDEX);
}

template<>
int to<int>(lua_State* L, int index);

template<>
bool to<bool>(lua_State* L, int index);

template<>
std::string to<std::string>(lua_State* L, int index);

template<>
TimeDirection to<TimeDirection>(lua_State* L, int index);

template<>
Wall to<Wall>(lua_State* L, int index);

template<>
Environment to<Environment>(lua_State* L, int index);

template<>
InitialBox to<InitialBox>(lua_State* L, int index);

template<>
InitialObjects to<InitialObjects>(lua_State* L, int index);

template<>
InitialGuyArrival to<InitialGuyArrival>(lua_State* L, int index);

template<>
Ability to<Ability>(lua_State* L, int index);

//For pickups
//I should probably make a Pickup class to make this less ambiguous.
template<>
mt::std::map<Ability, int>::type
    to<mt::std::map<Ability, int>::type>(lua_State* L, int index);

template<>
Guy to<Guy>(lua_State* L, int index);

template<>
InitialGuy to<InitialGuy>(lua_State* L, int index);

template<>
TriggerSystem to<TriggerSystem>(lua_State* L, int index);

template<>
unique_ptr<DirectLuaTriggerSystem> to<unique_ptr<DirectLuaTriggerSystem> >(lua_State* L, int index);

template<>
TriggerOffsetsAndDefaults to<TriggerOffsetsAndDefaults>(lua_State* L, int index);
}
#endif //HG_LUA_UTILITIES_H
