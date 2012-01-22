#include "LuaUtilities.h"
#include <fstream>
#include <iostream>
#include "lua/lualib.h"
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/range/algorithm/find_if.hpp>

//TODO everywhere:
//* fix usage of lua_checkstack
//* make code correct in the face of errors and incorrect input
//   (apply RAII, throw exceptions consistently on bad input, make the lua execution be interuptable)
namespace hg {
std::vector<char> loadFileIntoVector(
    std::string const& filename,
    OperationInterruptor const& interruptor)
{
    std::vector<char> vec;
    std::ifstream file;
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
    file.open(filename.c_str());
    file.seekg(0, std::ios::end);
    std::streampos length(file.tellg());
    if (length) {
        file.seekg(0, std::ios::beg);
        vec.resize(static_cast<std::size_t>(length));
        file.read(&vec.front(), static_cast<std::size_t>(length));
    }
    return vec;
}

int lua_VectorWriter(
    lua_State *L,
    const void* p,
    size_t sz,
    void* ud)
{
    (void)L;
    std::vector<char>& vec(*static_cast<std::vector<char>*>(ud));
    vec.insert(vec.end(), static_cast<char const*>(p), static_cast<char const*>(p) + sz);
    return 0;
}

const char * lua_VectorReader (
    lua_State *L,
    void *ud,
    size_t *size)
{
    (void)L;
    std::pair<char const*, char const*>& data(*static_cast<std::pair<char const*, char const*>*> (ud));
    if (data.first != data.second) {
        const char* retv(data.first);
        *size = data.second - data.first;
        data.first = data.second;
        return retv;
    }
    else {
        return 0;
    }
}
//Doesn't quite work...
template<typename InputIterator>
const char * lua_InputIteratorReader (
    lua_State *L,
    void *ud,
    size_t *size)
{
    (void)L;
    InputIterator& it(*static_cast<InputIterator*>(ud));
    
}

static char const* safe_functions[] = {
"assert",
"error",
"ipairs",
"next",
"pairs",
"pcall",
"select",
"tonumber",
"tostring",
"type",
"unpack",
"_VERSION",
"xpcall",
"coroutine.create",
"coroutine.resume",
"coroutine.running",
"coroutine.status",
"coroutine.wrap",
"coroutine.yield",//TODO - properly investigate the security implications of allowing this
"string.byte",
"string.char",
"string.find",
"string.format",
"string.gmatch",
"string.gsub",
"string.len",
"string.lower"
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

struct CStringEqual {
    CStringEqual(char const* string) : string_(string) {}
    bool operator()(char const* otherString) const {
        return std::strcmp(otherString, string_) == 0;
    }
    private:
    char const* string_;
};

static bool namesSafeFunction(char const* string) {
    return boost::find_if(safe_functions, CStringEqual(string));
}

//lua_CFunction which takes one argument and returns one value.
//Its first upvalue is taken to be a global environment which has had libraries loaded into it.
//Elements of that environment are assumed to be the standard lua library functions
//if they have the same names as standard lua functions.
//The indexed value is returned if it is a safe function for use in a sandbox
//(ie, a safe standard function, or a safe custom function).
//Tables (ie - packages) are not returned directly
//(because that would allow the package to be modified - an operation which would leak information),
//but if the argument uses the "table.function" form, the function may be returned.
//Currently there is no support for the [=[table["function"]]=] form, sorry.

//This function is meant to be made the single element of the global
//environment, to allow the environment to be easily be sandboxed and cleansed.

//Hardcoded support for only two forms:
//"function"
//"package.function"
//It may be worth putting in more support later.
static int get_sandboxed_function(lua_State* L)
{
    assert(lua_gettop(L) == 1);
    assert(lua_type(L, 1) == LUA_TSTRING);
    size_t length;
    char const* string(lua_tolstring(L, 1, &length));
    if (namesSafeFunction(string)) {
        using boost::algorithm::split_iterator;
        using boost::algorithm::first_finder;
        int firstTime(true);
        for (split_iterator<char const*> it(string, string + length, first_finder(".")); !it.eof(); ++it)
        {
            lua_pushlstring(L, boost::begin(*it), boost::distance(*it));
            lua_gettable(L, firstTime ? lua_upvalueindex(1) : -2);
            firstTime = false;
        }
        assert(lua_type(L, -1) == LUA_TFUNCTION);
        lua_replace(L, 1);
        lua_pop(L, lua_gettop(L) - 1);
        assert(lua_type(L, 1) == LUA_TFUNCTION);
    }
    else {
        assert(false);
        lua_pushnil(L);
    }
    return 1;
}

void loadSandboxedLibraries(lua_State* L)
{
    //Load all libs
    //the io, debug and os libs are not used, so they should probably not be loaded.
    luaL_openlibs(L);
    //Create the "get" function and load it into the registry
    //[
    //Create the "HourglassII" table
    lua_createtable(L, 0, 1);
    //Create the "get" function and load it into the "HourglassII" table
    lua_pushnumber(L, LUA_RIDX_GLOBALS);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_pushcclosure(L, get_sandboxed_function, 1);
    lua_setfield(L, -2, "get");
    //Put the "HourglassII" table into the registry
    lua_setfield(L, LUA_REGISTRYINDEX, "HourglassII");
    //]
}
//Creates a new table containing (as its single member) a function at index "get"
//which can be called with the names of functions, and which returns the named function if
//the name is the name of a function which is safe to call from a sandboxed environment.

//Assigns this new table to both _ENV upvalue of the function at the given index, and the global environment.
void sandboxFunction(lua_State* L, int index)
{
#ifndef NDEBUG
    int initialStackSize(lua_gettop(L));
#endif
    lua_pushvalue(L, index);//1{farg}
    lua_pushinteger(L, LUA_RIDX_GLOBALS);//2{farg,iglob}
    lua_createtable(L, 0, 1);//3{farg,iglob,tnew}
    lua_getfield(L, LUA_REGISTRYINDEX, "HourglassII");//4{farg,iglob,tnew,tHGII{get}}
    lua_getfield(L, -1, "get");//5{farg,iglob,tnew,tHGII{get},fget}
    lua_remove(L, -2);//4{farg,iglob,tnew,fget}
    lua_setfield(L, -2, "get");//3{farg,iglob,tnew{get}}
    lua_pushvalue(L, -1);//4{farg,iglob,tnew{get},tnew{get}}
    lua_setupvalue(L, -4, 1);//3{farg,iglob,tnew{get}}
    lua_settable(L, LUA_REGISTRYINDEX);//1{farg}
    lua_pop(L, 1);//0{}
#ifndef NDEBUG
    int finalStackSize(lua_gettop(L));
#endif
    assert(initialStackSize == finalStackSize);
}

LuaState loadLuaStateFromVector(std::vector<char> const& luaData, std::string const& chunkName)
{
    std::pair<char const*, char const*> source_iterators;
    if (!luaData.empty()) {
        source_iterators.first = &luaData.front();
        source_iterators.second = &luaData.front() + luaData.size();
    }
    LuaState L((LuaState::new_state_t()));
    if (lua_load(L.ptr, lua_VectorReader, &source_iterators, chunkName.c_str(), 0) != LUA_OK) {
        std::cerr << lua_tostring(L.ptr, -1) << std::endl;
        assert(false);
    }
    assert(lua_type(L.ptr, -1) == LUA_TFUNCTION);
    return boost::move(L);
}

template<>
bool isValid<bool>(lua_State* L, int index)
{
    return lua_isboolean(L, index);
}

template<>
bool to<bool>(lua_State* L, int index)
{
    //TODO: better error checking
    assert(lua_isboolean(L, index));
    return lua_toboolean(L, index);
}
template<>
bool isValid<int>(lua_State* L, int index)
{
    return lua_isnumber(L, index);
}
template<>
int to<int>(lua_State* L, int index)
{
    //TODO: better rounding/handling of non-integers!
    assert(lua_isnumber(L, index));
    return static_cast<int>(lua_tointeger(L, index));
}

template<>
std::string to<std::string>(lua_State* L, int index)
{
    assert(lua_isstring(L, index));
    return std::string(lua_tostring(L, index));
}

template<>
TimeDirection to<TimeDirection>(lua_State* L, int index)
{
    assert(lua_isstring(L, index));
    char const* timeDirectionString(lua_tostring(L, index));
    TimeDirection retv(INVALID);
    if (strcmp(timeDirectionString, "forwards") == 0) {
        retv = FORWARDS;
    }
    else if (strcmp(timeDirectionString, "reverse") == 0) {
        retv = REVERSE;
    }
    else {
    	std::cerr << timeDirectionString << std::endl;
    	assert(false && "invalid string given as timeDirection");
    }
    return retv;
}

template<>
Wall to<Wall>(lua_State* L, int index)
{
    int segmentSize(readField<int>(L, "segmentSize", index));
    int width(readField<int>(L, "width", index));
    assert(width >= 0);
    int height(readField<int>(L, "height", index));
    assert(height >= 0);
    
    boost::array<boost::multi_array<bool, 2>::index, 2> const wallShape = {{ width, height }};
    boost::multi_array<bool, 2> wall(wallShape);
    
    assert(lua_rawlen(L, index) == static_cast<unsigned>(height));
    for (std::size_t i(1), iend(height); i <= iend; ++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, index - 1);
        assert(lua_rawlen(L, -1) == static_cast<unsigned>(width));
        for (std::size_t j(1), jend(width); j <= jend;  ++j) {
            lua_pushinteger(L, j);
            lua_gettable(L, -2);
            wall[j - 1][i - 1] = to<int>(L);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }
    return Wall(segmentSize, wall);
}

template<>
Environment to<Environment>(lua_State* L, int index)
{
    return Environment(readField<Wall>(L, "wall", index), readField<int>(L, "gravity", index));
}

template<>
InitialBox to<InitialBox>(lua_State* L, int index)
{
    assert(lua_istable(L, index) && "an initial box must be a table");
    assert(readField<std::string>(L, "type", index) == "box" 
        && "there is no support for initial objects other than boxes");
    return
      InitialBox(
        Box(readField<int>(L, "x", index), readField<int>(L, "y", index),
            readField<int>(L, "xspeed", index), readField<int>(L, "yspeed", index),
            readField<int>(L, "size", index),
            -1,
            -1,
            readField<TimeDirection>(L, "timeDirection", index)));
}

template<>
InitialObjects to<InitialObjects>(lua_State* L, int index)
{
    ObjectList<NonGuyDynamic> retv;
    for (std::size_t i(1), iend(lua_rawlen(L, index)); i <= iend; ++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, index - 1);
        std::string type(readField<std::string>(L, "type"));
        if (type == "box") {
            retv.add(to<InitialBox>(L).box);            
        }
        else {
            std::cerr << type << std::endl;
            assert(false && "invalid type given for InitialObject");
        }
        lua_pop(L, 1);
    }
    return InitialObjects(retv);
}


template<>
InitialGuyArrival to<InitialGuyArrival>(lua_State* L, int index)
{
    return InitialGuyArrival(readField<int>(L, "arrivalTime", index), readField<InitialGuy>(L, "arrival", index).guy);
}


template<>
Ability to<Ability>(lua_State* L, int index)
{
    std::string abilityString(lua_tostring(L, index));
    if (abilityString == "timeJump") {
        return TIME_JUMP;
    }
    else if (abilityString == "timeReverse") {
        return TIME_REVERSE;
    }
    else if (abilityString == "timeGun") {
        return TIME_GUN;
    }
    else {
        std::cerr << abilityString << std::endl;
        assert(false && "invalid ability string");
        return NO_ABILITY;
    }
}

template<>
FacingDirection::FacingDirection to<FacingDirection::FacingDirection>(lua_State* L, int index)
{
    std::string facingString(lua_tostring(L, index));
    if (facingString == "left") {
        return FacingDirection::LEFT;
    }
    else if (facingString == "right") {
        return FacingDirection::RIGHT;
    }
    else {
        std::cerr << facingString << std::endl;
        assert(false && "invalid facing direction string");
    }
}

template<>
mt::std::map<Ability, int>::type
    to<mt::std::map<Ability, int>::type>(lua_State* L, int index)
{
    assert(lua_istable(L, index) && "pickups must be a table");
    mt::std::map<Ability, int>::type retv;
    lua_pushnil(L);
    while (lua_next(L, index - 1) != 0) {
        lua_Integer abilityQuantity(lua_tointeger(L, -1));
        assert(abilityQuantity >= -1);
        retv[to<Ability>(L, -2)] = static_cast<int>(abilityQuantity);
        lua_pop(L, 1);
    }
    return retv;
}

template<>
Guy to<Guy>(lua_State* L, int index)
{
    assert(lua_istable(L, index) && "a guy must be a table");
    int relativeIndex(readField<int>(L, "index",index));
    int x(readField<int>(L, "x",index));            
    int y(readField<int>(L, "y",index));
    int xspeed(readField<int>(L, "xspeed",index));
    int yspeed(readField<int>(L, "yspeed",index));
    int width(readField<int>(L, "width",index));
    int height(readField<int>(L, "height",index));
    int illegalPortal(-1);
    lua_getfield(L, index, "illegalPortal");
    if (!lua_isnil(L, -1)) {
        assert(lua_isnumber(L, -1));
        illegalPortal = static_cast<int>(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    int arrivalBasis(-1);
    lua_getfield(L, index, "arrivalBasis");
    if (!lua_isnil(L, -1)) {
        arrivalBasis = to<int>(L);
    }
    lua_pop(L, 1);
    bool supported(readField<bool>(L, "supported", index));
    int supportedSpeed(0);
    if (supported) {
        supportedSpeed = readField<int>(L, "supportedSpeed", index);
    }
    mt::std::map<Ability, int>::type pickups(readField<mt::std::map<Ability, int>::type>(L, "pickups", index));
    FacingDirection::FacingDirection facing(readField<FacingDirection::FacingDirection>(L, "facing", index));
    bool boxCarrying(readField<bool>(L, "boxCarrying", index));
    int boxCarrySize(0);
    TimeDirection boxCarryDirection(INVALID);
    if (boxCarrying) {
        boxCarrySize = readField<int>(L, "boxCarrySize", index);
        boxCarryDirection = readField<TimeDirection>(L, "boxCarryDirection", index);
    }
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection", index));
    return Guy(
        relativeIndex,
        x, y,
        xspeed, yspeed,
        width, height,
        illegalPortal,
        arrivalBasis,
        supported,
        supportedSpeed,
        pickups,
        facing,
        boxCarrying,
        boxCarrySize,
        boxCarryDirection,
        timeDirection);
}

template<>
InitialGuy to<InitialGuy>(lua_State* L, int index)
{
    assert(lua_istable(L, index) && "an initial guy must be a table");
    int x(readField<int>(L, "x",index));
    int y(readField<int>(L, "y",index));
    int xspeed(readFieldWithDefault<int>(L, "xspeed",index,0));
    int yspeed(readFieldWithDefault<int>(L, "yspeed",index,0));
    int width(readFieldWithDefault<int>(L, "width",index,1600));
    int height(readFieldWithDefault<int>(L, "height",index,3200));
    mt::std::map<Ability, int>::type pickups(readField<mt::std::map<Ability, int>::type>(L, "pickups", index));
    FacingDirection::FacingDirection facing(readField<FacingDirection::FacingDirection>(L, "facing", index));
    bool boxCarrying(readFieldWithDefault<bool>(L, "boxCarrying", index, false));
    int boxCarrySize(0);
    TimeDirection boxCarryDirection(INVALID);
    if (boxCarrying) {
        boxCarrySize = readField<int>(L, "boxCarrySize", index);
        boxCarryDirection = readField<TimeDirection>(L, "boxCarryDirection", index);
    }
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection", index));
    return
    InitialGuy(
      Guy(
        0,
        x, y,
        xspeed, yspeed,
        width, height,
        -1,
        -1,
        false,
        0,
        pickups,
        facing,
        boxCarrying,
        boxCarrySize,
        boxCarryDirection,
        timeDirection));
}

template<>
TriggerSystem to<TriggerSystem>(lua_State* L, int index)
{
    std::string type(readField<std::string>(L, "type", index));
    if (type == "DirectLua") {
        return TriggerSystem(to<unique_ptr<DirectLuaTriggerSystem> >(L).release());
    }
    else {
        std::cerr << type << std::endl;
        assert(false && "unrecognised triggerSystem type");
    }
}

template<>
unique_ptr<DirectLuaTriggerSystem> to<unique_ptr<DirectLuaTriggerSystem> >(lua_State* L, int index)
{
    std::string system(readField<std::string>(L, "system", index));
    return
      unique_ptr<DirectLuaTriggerSystem>(
        new DirectLuaTriggerSystem(
          std::vector<char>(system.begin(), system.end()),
          readField<TriggerOffsetsAndDefaults>(L, "triggerOffsetsAndDefaults", index).toad,
          readField<int>(L, "arrivalLocationsSize", index)));
}

template<>
TriggerOffsetsAndDefaults to<TriggerOffsetsAndDefaults>(lua_State* L, int index)
{
    std::vector<std::pair<int, std::vector<int> > > toad;
    std::size_t iend(lua_rawlen(L, index));
    toad.reserve(iend);
    for (std::size_t i(1); i <= iend; ++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, index - 1);
        int offset(readField<int>(L, "offset", index));
        
        lua_getfield(L, index, "default");
        std::vector<int> default_;
        std::size_t jend(lua_rawlen(L, -1));
        default_.reserve(jend);
        for (std::size_t j(1); j <= jend;  ++j) {
            lua_pushinteger(L, j);
            lua_gettable(L, -2);
            default_.push_back(to<int>(L));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        toad.push_back(std::make_pair(offset, default_));
        lua_pop(L, 1);
    }
    return TriggerOffsetsAndDefaults(toad);
}

} //namespace hg
