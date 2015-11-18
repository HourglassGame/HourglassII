#include "LuaUtilities.h"
#include "LuaUserData.h"
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "multi_vector.h"

#include "ImageGlitz.h"
#include "TextGlitz.h"
#include "LineGlitz.h"
#include "RectangleGlitz.h"

#include "LuaStackManager.h"
#include <cassert>
#define luaassert assert
//TODO everywhere:
//* fix usage of lua_checkstack
//* make code correct in the face of errors and incorrect input
//   (apply RAII, throw exceptions consistently on bad input, make the lua execution be interuptable)
namespace hg {
std::vector<char> loadFileIntoVector(
    boost::filesystem::path const &filename)
{
    std::vector<char> vec;
    boost::filesystem::ifstream file;
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
    file.open(filename, std::ifstream::in | std::ifstream::binary);
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
    const void *p,
    size_t sz,
    void *ud)
{
    (void)L;
    std::vector<char> &vec(*static_cast<std::vector<char> *>(ud));
    vec.insert(vec.end(), static_cast<char const *>(p), static_cast<char const *>(p) + sz);
    return 0;
}

const char *lua_VectorReader (
    lua_State *L,
    void *ud,
    size_t *size)
{
    (void)L;
    std::pair<char const *, char const *>& data(*static_cast<std::pair<char const *, char const *> *>(ud));
    if (data.first != data.second) {
        const char *retv(data.first);
        *size = data.second - data.first;
        data.first = data.second;
        return retv;
    }
    else {
        return nullptr;
    }
}


void pushFunctionFromVector(lua_State *L, std::vector<char> const &luaData, std::string const &chunkName)
{
    if (luaL_loadbuffer(L, luaData.empty() ? "" : &luaData.front() , luaData.size(), chunkName.c_str()) != LUA_OK) {
        std::cerr << lua_tostring(L, -1) << std::endl;
        assert(false);
    }
    assert(lua_type(L, -1) == LUA_TFUNCTION);
}

void checkstack(lua_State *L, int extra)
{
    if (!lua_checkstack(L, extra)) {
        //Maybe change to a more specific error?
        //I don't see hourglass ever overrunning the normal
        //stack space limit though.
        throw std::bad_alloc();
    }
}

template<>
bool isValid<bool>(lua_State *L, int index)
{
    return lua_isboolean(L, index);
}

template<>
bool to<bool>(lua_State *L, int index)
{
    //TODO: better error checking
    assert(lua_isboolean(L, index));
    return lua_toboolean(L, index);
}
template<>
bool isValid<int>(lua_State *L, int index)
{
    return lua_isnumber(L, index);
}
template<>
int to<int>(lua_State *L, int index)
{
    //TODO: better rounding/handling of non-integers!
    assert(lua_isnumber(L, index));
    lua_Number num = lua_tonumber(L, index);

    lua_Integer integ = 0;
    int is_integer = lua_numbertointeger(num, &integ);
    assert(is_integer);
    //Note that by default, lua_Integer has a much larger range than int,
    //so this conversion could overflow. This need to be improved.
    return static_cast<int>(integ);
}

template<>
std::string to<std::string>(lua_State *L, int index)
{
    assert(lua_isstring(L, index));
    return std::string(lua_tostring(L, index));
}

template<>
std::vector<std::string> to<std::vector<std::string> >(lua_State *L, int index) {
    
    assert(lua_istable(L, index));
    lua_len(L, index);
    int const tablelen(lua_tointeger(L, -1));
    lua_pop(L, 1);
    std::vector<std::string> retv(tablelen);
    for (int i(0), end(tablelen); i != end ; ++i) {
        lua_rawgeti(L, index, i+1);
        retv[i] = to<std::string>(L, -1);
        lua_pop(L, 1);
    }
    return retv;
}

template<>
TimeDirection to<TimeDirection>(lua_State *L, int index)
{
    assert(lua_isstring(L, index));
    char const *timeDirectionString(lua_tostring(L, index));
    TimeDirection retv{TimeDirection::INVALID};
    if (strcmp(timeDirectionString, "forwards") == 0) {
        retv = TimeDirection::FORWARDS;
    }
    else if (strcmp(timeDirectionString, "reverse") == 0) {
        retv = TimeDirection::REVERSE;
    }
    else {
        std::cerr << timeDirectionString << std::endl;
        assert(false && "invalid string given as timeDirection");
    }
    return retv;
}

template<>
Wall to<Wall>(lua_State *L, int index)
{
    int segmentSize(readField<int>(L, "segmentSize", index));
    int width(readField<int>(L, "width", index));
    assert(width >= 0);
    int height(readField<int>(L, "height", index));
    assert(height >= 0);
    
    std::array<hg::multi_vector<bool, 2>::index, 2> const wallShape = {{
        static_cast<hg::multi_vector<bool, 2>::index>(width),
        static_cast<hg::multi_vector<bool, 2>::index>(height)}};
    hg::multi_vector<bool, 2> wall;
    wall.resize(wallShape);
    
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
    return Wall(segmentSize, std::move(wall), "HourglassI");
}

template<>
Environment to<Environment>(lua_State *L, int index)
{
    return {readField<Wall>(L, "wall", index), readField<int>(L, "gravity", index)};
}

template<>
InitialBox to<InitialBox>(lua_State *L, int index)
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
InitialObjects to<InitialObjects>(lua_State *L, int index)
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
InitialGuyArrival to<InitialGuyArrival>(lua_State *L, int index)
{
    return InitialGuyArrival(readField<int>(L, "arrivalTime", index), readField<InitialGuy>(L, "arrival", index).guy);
}


template<>
Ability to<Ability>(lua_State *L, int index)
{
    std::string abilityString(lua_tostring(L, index));
    if (abilityString == "timeJump") {
        return Ability::TIME_JUMP;
    }
    else if (abilityString == "timeReverse") {
        return Ability::TIME_REVERSE;
    }
    else if (abilityString == "timeGun") {
        return Ability::TIME_GUN;
    }
    else if (abilityString == "timePause") {
        return Ability::TIME_PAUSE;
    }
    else {
        std::cerr << abilityString << std::endl;
        assert(false && "invalid ability string");
        return Ability::NO_ABILITY;
    }
}

template<>
FacingDirection to<FacingDirection>(lua_State *L, int index)
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
mt::std::map<Ability, int>
    to<mt::std::map<Ability, int>>(lua_State *L, int index)
{
    assert(lua_istable(L, index) && "pickups must be a table");
    mt::std::map<Ability, int> retv;
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
    int jumpSpeed(readField<int>(L, "jumpSpeed",index));
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
    mt::std::map<Ability, int> pickups(readField<mt::std::map<Ability, int>>(L, "pickups", index));
    FacingDirection facing(readField<FacingDirection>(L, "facing", index));
    bool boxCarrying(readField<bool>(L, "boxCarrying", index));
    int boxCarrySize(0);
    TimeDirection boxCarryDirection{TimeDirection::INVALID};
    if (boxCarrying) {
        boxCarrySize = readField<int>(L, "boxCarrySize", index);
        boxCarryDirection = readField<TimeDirection>(L, "boxCarryDirection", index);
    }
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection", index));
    bool timePaused(readField<bool>(L, "timePaused", index));
    return Guy(
        relativeIndex,
        x, y,
        xspeed, yspeed,
        width, height,
        jumpSpeed,
        illegalPortal,
        arrivalBasis,
        supported,
        supportedSpeed,
        pickups,
        facing,
        boxCarrying,
        boxCarrySize,
        boxCarryDirection,
        timeDirection,
        timePaused);
}

template<>
InitialGuy to<InitialGuy>(lua_State *L, int index)
{
    assert(lua_istable(L, index) && "an initial guy must be a table");
    int x(readField<int>(L, "x",index));
    int y(readField<int>(L, "y",index));
    int xspeed(readFieldWithDefault<int>(L, "xspeed",index,0));
    int yspeed(readFieldWithDefault<int>(L, "yspeed",index,0));
    int width(readFieldWithDefault<int>(L, "width",index,1600));
    int height(readFieldWithDefault<int>(L, "height",index,3200));
    int jumpSpeed(readFieldWithDefault<int>(L, "jumpSpeed",index,-450));
    mt::std::map<Ability, int> pickups(readField<mt::std::map<Ability, int>>(L, "pickups", index));
    FacingDirection facing(readField<FacingDirection>(L, "facing", index));
    bool boxCarrying(readFieldWithDefault<bool>(L, "boxCarrying", index, false));
    int boxCarrySize(0);
    TimeDirection boxCarryDirection{TimeDirection::INVALID};
    if (boxCarrying) {
        boxCarrySize = readField<int>(L, "boxCarrySize", index);
        boxCarryDirection = readField<TimeDirection>(L, "boxCarryDirection", index);
    }
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection", index));
    bool timePaused(readFieldWithDefault<bool>(L, "timePaused", index, false));
    return
    InitialGuy(
      Guy(
        0,
        x, y,
        xspeed, yspeed,
        width, height,
        jumpSpeed,
        -1,
        -1,
        false,
        0,
        pickups,
        facing,
        boxCarrying,
        boxCarrySize,
        boxCarryDirection,
        timeDirection,
        timePaused));
}

template<>
TriggerOffsetsAndDefaults to<TriggerOffsetsAndDefaults>(lua_State *L, int index)
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
    return {toad};
}

template<>
Collision to<Collision>(lua_State *L, int index)
{
    assert(lua_istable(L, -1) && "a collision must be a table");

    int x(readField<int>(L, "x", index));            
    int y(readField<int>(L, "y", index));
    int xspeed(readField<int>(L, "xspeed", index));
    int yspeed(readField<int>(L, "yspeed", index));
    int width(readField<int>(L, "width", index));
    int height(readField<int>(L, "height", index));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection", index));
    
    return Collision(x, y, xspeed, yspeed, width, height, timeDirection);
}

template<>
Glitz to<Glitz>(lua_State *L, int index)
{
    std::string const type(readField<std::string>(L, "type", index));
    if (type == "rectangle") {
        int const layer(readField<int>(L, "layer", index));
        int const x(readField<int>(L, "x", index));
        int const y(readField<int>(L, "y", index));
        int const width(readField<int>(L, "width", index));
        int const height(readField<int>(L, "height", index));
        unsigned colour(readColourField(L, "colour"));
        return Glitz(new (multi_thread_tag{}) RectangleGlitz(layer, x, y, width, height, colour));
    }
    else if (type == "text") {
        int const layer(readField<int>(L, "layer", index));
        std::string text(readField<std::string>(L, "text", index));
        int const x(readField<int>(L, "x", index));
        int const y(readField<int>(L, "y", index));
        int const size(readField<int>(L, "size", index));
        unsigned colour(readColourField(L, "colour"));
        return Glitz(new (multi_thread_tag{}) TextGlitz(layer, std::move(text), x, y, size, colour));
    }
    else if (type == "image") {
        int const layer(readField<int>(L, "layer"));
        std::string key(readField<std::string>(L, "key"));
        int const x(readField<int>(L, "x"));
        int const y(readField<int>(L, "y"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        
        return Glitz(new (multi_thread_tag{}) ImageGlitz(layer, std::move(key), x, y, width, height));
    }
    std::cerr << "Unknown Glitz Type: " << type << "\n";
    luaassert("Unknown Glitz Type" && false);
}

unsigned readColourField(lua_State *L, char const *fieldName)
{
    LuaStackManager stack_manager(L);
    lua_getfield(L, -1, fieldName);
    unsigned r(readField<int>(L, "r"));
    unsigned g(readField<int>(L, "g"));
    unsigned b(readField<int>(L, "b"));
    lua_pop(L, 1);
    return r << 24 | g << 16 | b << 8;
}


} //namespace hg
