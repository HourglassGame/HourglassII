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
#include "OperationInterrupter.h"
#include <boost/filesystem/path.hpp>

//Miscelaneous things related to lua that are useful in multiple places
//Should be replaced by a more extensible and cohesive model eventually
namespace hg {
char const *lua_VectorReader (
    lua_State *L,
    void *ud,
    size_t *size);
int lua_VectorWriter(
    lua_State *L,
    void const *p,
    size_t sz,
    void *ud);
std::vector<char> loadFileIntoVector(boost::filesystem::path const &filename);

void pushFunctionFromVector(lua_State *L, std::vector<char> const &luaData, std::string const &chunkName);

//Identical to lua_checkstack, but throws on failure.
void checkstack(lua_State *L, int extra);

template<typename T>
T to(lua_State *L, int index = -1);

template<typename T>
bool isValid(lua_State *L, int index = -1);

template<typename T>
T toWithDefault(lua_State *L, int index, T defaultValue)
{
    return isValid<T>(L, index) ? to<T>(L, index) : defaultValue;
}

template<typename T>
T readFieldWithDefault(lua_State *L, char const *fieldName, int index, T defaultValue)
{
    lua_getfield(L, index, fieldName);
    T retv(toWithDefault<T>(L, -1, defaultValue));
    lua_pop(L, 1);
    return retv;
}

template<typename T>
T readField(lua_State *L, char const *fieldName, int index = -1)
{
    lua_getfield(L, index, fieldName);
    T retv(to<T>(L));
    lua_pop(L, 1);
    return retv;
}
template<typename T>
T readGlobal(lua_State *L, char const *globalName)
{
    lua_getglobal(L, globalName);
    T retv(to<T>(L));
    lua_pop(L, 1);
    return retv;
}

template<>
int to<int>(lua_State *L, int index);

template<>
bool isValid<int>(lua_State *L, int index);

template<>
bool to<bool>(lua_State *L, int index);

template<>
std::string to<std::string>(lua_State *L, int index);

template<>
std::vector<std::string> to<std::vector<std::string> >(lua_State *L, int index);

template<>
TimeDirection to<TimeDirection>(lua_State *L, int index);

template<>
Wall to<Wall>(lua_State *L, int index);

template<>
Environment to<Environment>(lua_State *L, int index);

template<>
InitialBox to<InitialBox>(lua_State *L, int index);

template<>
InitialObjects to<InitialObjects>(lua_State *L, int index);

template<>
InitialGuyArrival to<InitialGuyArrival>(lua_State *L, int index);

template<>
Ability to<Ability>(lua_State *L, int index);

template<>
FacingDirection to<FacingDirection>(lua_State *L, int index);

//For pickups
//I should probably make a Pickup class to make this less ambiguous.
template<>
mt::std::map<Ability, int>
    to<mt::std::map<Ability, int>>(lua_State *L, int index);

template<>
Guy to<Guy>(lua_State *L, int index);

template<>
InitialGuy to<InitialGuy>(lua_State *L, int index);

template<>
unique_ptr<DirectLuaTriggerSystem> to<unique_ptr<DirectLuaTriggerSystem> >(lua_State *L, int index);

template<>
TriggerOffsetsAndDefaults to<TriggerOffsetsAndDefaults>(lua_State *L, int index);

template<>
Collision to<Collision>(lua_State *L, int index);
}
#endif //HG_LUA_UTILITIES_H
