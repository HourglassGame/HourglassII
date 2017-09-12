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
    //Need to use binary mode; otherwise CRLF line endings count as 2 for
    //`length` calculation but only 1 for `file.read` (on some platforms),
    //and we get undefined  behaviour when trying to read `length` characters.
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
    lua_State * const L,
    const void * const p,
    size_t const sz,
    void * const ud)
{
    (void)L;
    std::vector<char> &vec(*static_cast<std::vector<char> *>(ud));
    vec.insert(vec.end(), static_cast<char const *>(p), static_cast<char const *>(p) + sz);
    return 0;
}

const char *lua_VectorReader (
    lua_State * const L,
    void * const ud,
    size_t * const size)
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

void pushFunctionFromVector(lua_State * const L, std::vector<char> const &luaData, std::string const &chunkName)
{
    if (luaL_loadbuffer(L, luaData.empty() ? "" : &luaData.front() , luaData.size(), chunkName.c_str()) != LUA_OK) {
        std::stringstream ss;
        ss << "Couldn't load lua-function: " << lua_tostring(L, -1);
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
    }
    assert(lua_type(L, -1) == LUA_TFUNCTION && "luaL_loadbuffer is specified to push a function onto the lua-stack if it succeeds");
}

void checkstack(lua_State * const L, int const extra)
{
    if (!lua_checkstack(L, extra)) {
        //Maybe change to a more specific error?
        //I don't see hourglass ever overrunning the normal
        //stack space limit though.
        throw std::bad_alloc();
    }
}

template<>
bool isValid<bool>(lua_State * const L, int const index)
{
    return lua_isboolean(L, index);
}

template<>
bool to<bool>(lua_State * const L, int const index) {
    try
    {
        //TODO: better error checking
        if (!lua_isboolean(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Non-boolean value produced where bool expected"));
        }
        return lua_toboolean(L, index);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<bool>");
        throw;
    }
}
template<>
bool isValid<int>(lua_State * const L, int const index)
{
    return lua_isnumber(L, index);
}
template<>
int to<int>(lua_State * const L, int const index) {
    try
    {
        //TODO: better rounding/handling of non-integers!
        if (!lua_isnumber(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Non-numeric value found where numeric value expected"));
        }
        lua_Number const num = lua_tonumber(L, index);

        lua_Integer integ = 0;

        int const is_integer = lua_numbertointeger(num, &integ);
        if (!is_integer) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Numeric value out-of-range to be an integer, unable to convert"));
        }
        //Note that by default, lua_Integer has a much larger range than int,
        //so this conversion could overflow. This need to be improved.
        return static_cast<int>(integ);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<int>");
        throw;
    }
}
template<>
std::string to<std::string>(lua_State * const L, int const index) {
    try
    {
        if (!lua_isstring(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Non-string value found where string value expected"));
        }
        return std::string(lua_tostring(L, index));
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<std::string>");
        throw;
    }
}

template<>
mt::std::string to<mt::std::string>(lua_State * const L, int const index) {
    try
    {
        if (!lua_isstring(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Non-string value found where string value expected"));
        }
        return mt::std::string(lua_tostring(L, index));
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<mt::std::string>");
        throw;
    }
}

template<>
std::vector<std::string> to<std::vector<std::string> >(lua_State * const L, int const index) {
    try {
        if (!lua_istable(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Non-table value found where table value expected"));
        }
        lua_len(L, index);
        lua_Integer const tablelen(lua_tointeger(L, -1));
        lua_pop(L, 1);
        std::vector<std::string> retv(tablelen);
        for (lua_Integer i(0), end(tablelen); i != end; ++i) {
            lua_rawgeti(L, index, i + 1);
            retv[i] = to<std::string>(L, -1);
            lua_pop(L, 1);
        }
        return retv;
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<std::vector<std::string>>");
        throw;
    }
}

template<>
TimeDirection to<TimeDirection>(lua_State * const L, int const index) {
    try {
        if (!lua_isstring(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("TimeDirection values must be strings"));
        }
        char const * const timeDirectionString(lua_tostring(L, index));
        if (strcmp(timeDirectionString, "forwards") == 0) {
            return TimeDirection::FORWARDS;
        }
        else if (strcmp(timeDirectionString, "reverse") == 0) {
            return TimeDirection::REVERSE;
        }
        else {
            std::stringstream ss;
            ss << "Invalid string given as timeDirection: " << timeDirectionString;
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
        }
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<TimeDirection>");
        throw;
    }
}

template<>
Wall to<Wall>(lua_State * const L, int const index) {
    try
    {
        int const segmentSize(readField<int>(L, "segmentSize", index));
        int const width(readField<int>(L, "width", index));
        if (width < 0) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Wall width must be >= 0"));
        }
        int const height(readField<int>(L, "height", index));
        if (height < 0) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Wall height must be >= 0"));
        }

        std::array<hg::multi_vector<bool, 2>::index, 2> const wallShape = { {
            static_cast<hg::multi_vector<bool, 2>::index>(width),
            static_cast<hg::multi_vector<bool, 2>::index>(height)} };
        hg::multi_vector<bool, 2> wall;
        wall.resize(wallShape);

        if (lua_rawlen(L, index) != static_cast<unsigned>(height)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Wall table size doesn't match specified wall height"));
        }
        for (std::size_t i(1), iend(height); i <= iend; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, index - 1);

            if (lua_rawlen(L, -1) != static_cast<unsigned>(width)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Wall table size doesn't match specified wall width"));
            }
            for (std::size_t j(1), jend(width); j <= jend; ++j) {
                lua_pushinteger(L, j);
                lua_gettable(L, -2);
                wall[j - 1][i - 1] = to<int>(L);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        return Wall(segmentSize, std::move(wall), "HourglassI");
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Wall>");
        throw;
    }
}
template<>
Environment to<Environment>(lua_State *L, int index) {
    try
    {
        return { readField<Wall>(L, "wall", index), readField<int>(L, "gravity", index) };
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Environment>");
        throw;
    }
}

template<>
InitialBox to<InitialBox>(lua_State *L, int index) {
    try {
        if (!lua_istable(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("an initial box must be a table"));
        }
        if (readField<std::string>(L, "type", index) != "box") {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("there is no support for initial objects other than boxes"));
        }
        return
            InitialBox(
                Box(readField<int>(L, "x", index), readField<int>(L, "y", index),
                    readField<int>(L, "xspeed", index), readField<int>(L, "yspeed", index),
                    readField<int>(L, "size", index),
                    -1,
                    -1,
                    readField<TimeDirection>(L, "timeDirection", index)));
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<InitialBox>");
        throw;
    }
}


template<>
InitialObjects to<InitialObjects>(lua_State *L, int index) {
    try {
        ObjectList<NonGuyDynamic> retv;
        for (std::size_t i(1), iend(lua_rawlen(L, index)); i <= iend; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, index - 1);

            lua_getfield(L, index, "type");
            if (!lua_isstring(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("InitialObject[n].type values must be strings"));
            }
            char const * const type(lua_tostring(L, -1));

            if (strcmp(type, "box") == 0) {
                lua_pop(L, 1);
                retv.add(to<InitialBox>(L).box);
            }
            else {
                std::stringstream ss;
                ss << "invalid type given for InitialObject: " << type;
                lua_pop(L, 1);
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
            }
            lua_pop(L, 1);
        }
        return InitialObjects(retv);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<InitialObjects>");
        throw;
    }
}

template<>
InitialGuyArrival to<InitialGuyArrival>(lua_State *L, int index) {
    try
    {
        return InitialGuyArrival(readField<int>(L, "arrivalTime", index), readField<InitialGuy>(L, "arrival", index).guy);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<InitialGuyArrival>");
        throw;
    }
}


template<>
Ability to<Ability>(lua_State *L, int index) {
    try {
        //TODO abstract and combine this with to<TimeDirection> and to<FacingDirection> (maybe)
        if (!lua_isstring(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Ability values must be strings"));
        }
        char const * const abilityString(lua_tostring(L, index));
        if (strcmp(abilityString, "timeJump") == 0) {
            return Ability::TIME_JUMP;
        }
        else if (strcmp(abilityString, "timeReverse") == 0) {
            return Ability::TIME_REVERSE;
        }
        else if (strcmp(abilityString, "timeGun") == 0) {
            return Ability::TIME_GUN;
        }
        else if (strcmp(abilityString, "timePause") == 0) {
            return Ability::TIME_PAUSE;
        }
        else {
            std::stringstream ss;
            ss << "invalid ability string: " << abilityString;
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
        }
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Ability>");
        throw;
    }
}

template<>
FacingDirection to<FacingDirection>(lua_State *L, int index) {
    try
    {
        if (!lua_isstring(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("FacingDirection values must be strings"));
        }
        char const * const facingString(lua_tostring(L, index));
        if (strcmp(facingString, "left") == 0) {
            return FacingDirection::LEFT;
        }
        else if (strcmp(facingString, "right") == 0) {
            return FacingDirection::RIGHT;
        }
        else {
            std::stringstream ss;
            ss << "invalid facing direction string: " << facingString;
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
        }
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<FacingDirection>");
        throw;
    }
}
template<>
mt::std::map<Ability, int>
to<mt::std::map<Ability, int>>(lua_State *L, int index) {
    try
    {
        if (!lua_istable(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("pickups must be a table"));
        }
        mt::std::map<Ability, int> retv;
        lua_pushnil(L);
        while (lua_next(L, index - 1) != 0) {
            auto abilityQuantity(std::round(lua_tonumber(L, -1)));
            if (abilityQuantity < -1) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("pickupQuantity must be an integer in the range [-1, inf)"));
            }
            retv[to<Ability>(L, -2)] = static_cast<int>(abilityQuantity);
            lua_pop(L, 1);
        }
        return retv;
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<InitialPickups>");
        throw;
    }
}

template<>
Guy to<Guy>(lua_State* L, int index) {
    try {
        if (!lua_istable(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a guy must be a table"));
        }
        int const relativeIndex(readField<int>(L, "index", index));
        int const x(readField<int>(L, "x", index));
        int const y(readField<int>(L, "y", index));
        int const xspeed(readField<int>(L, "xspeed", index));
        int const yspeed(readField<int>(L, "yspeed", index));
        int const width(readField<int>(L, "width", index));
        int const height(readField<int>(L, "height", index));
        int const jumpSpeed(readField<int>(L, "jumpSpeed", index));
        int illegalPortal(-1);
        lua_getfield(L, index, "illegalPortal");
        if (!lua_isnil(L, -1)) {
            if (!lua_isnumber(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Guy's illegalPortal must be nil or a number"));
            }
            illegalPortal = static_cast<int>(std::round(lua_tonumber(L, -1)));
        }
        lua_pop(L, 1);
        int arrivalBasis(-1);
        lua_getfield(L, index, "arrivalBasis");
        if (!lua_isnil(L, -1)) {
            arrivalBasis = to<int>(L);
        }
        lua_pop(L, 1);
        bool const supported(readField<bool>(L, "supported", index));
        int const supportedSpeed(supported ? readField<int>(L, "supportedSpeed", index) : 0);
        mt::std::map<Ability, int> pickups(readField<mt::std::map<Ability, int>>(L, "pickups", index));
        FacingDirection const facing(readField<FacingDirection>(L, "facing", index));
        bool const boxCarrying(readField<bool>(L, "boxCarrying", index));
        int const boxCarrySize(boxCarrying ? readField<int>(L, "boxCarrySize", index) : 0);
        TimeDirection const boxCarryDirection{ boxCarrying ? readField<TimeDirection>(L, "boxCarryDirection", index) : TimeDirection::INVALID };
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection", index));
        bool const timePaused(readField<bool>(L, "timePaused", index));
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
            std::move(pickups),
            facing,
            boxCarrying,
            boxCarrySize,
            boxCarryDirection,
            timeDirection,
            timePaused);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Guy>");
        throw;
    }
}

template<>
InitialGuy to<InitialGuy>(lua_State *L, int index) {
    try {
        if (!lua_istable(L, index)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("an initial guy must be a table"));
        }
        int const x(readField<int>(L, "x", index));
        int const y(readField<int>(L, "y", index));
        int const xspeed(readFieldWithDefault<int>(L, "xspeed", index, 0));
        int const yspeed(readFieldWithDefault<int>(L, "yspeed", index, 0));
        int const width(readFieldWithDefault<int>(L, "width", index, 1600));
        int const height(readFieldWithDefault<int>(L, "height", index, 3200));
        int const jumpSpeed(readFieldWithDefault<int>(L, "jumpSpeed", index, -450));
        mt::std::map<Ability, int> pickups(readField<mt::std::map<Ability, int>>(L, "pickups", index));
        FacingDirection const facing(readField<FacingDirection>(L, "facing", index));
        bool const boxCarrying(readFieldWithDefault<bool>(L, "boxCarrying", index, false));
        int const boxCarrySize(boxCarrying ? readField<int>(L, "boxCarrySize", index) : 0);
        TimeDirection const boxCarryDirection{ boxCarrying ? readField<TimeDirection>(L, "boxCarryDirection", index) : TimeDirection::INVALID };
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection", index));
        bool const timePaused(readFieldWithDefault<bool>(L, "timePaused", index, false));
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
                    std::move(pickups),
                    facing,
                    boxCarrying,
                    boxCarrySize,
                    boxCarryDirection,
                    timeDirection,
                    timePaused));
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<InitialGuy>");
        throw;
    }
}

template<>
TriggerOffsetsAndDefaults to<TriggerOffsetsAndDefaults>(lua_State *L, int index) {
    try {
        std::vector<std::pair<int, std::vector<int> > > toad;
        std::size_t const iend(lua_rawlen(L, index));
        toad.reserve(iend);
        for (std::size_t i(1); i <= iend; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, index - 1);
            int offset(readField<int>(L, "offset", index));

            lua_getfield(L, index, "default");
            std::vector<int> default_;
            std::size_t const jend(lua_rawlen(L, -1));
            default_.reserve(jend);
            for (std::size_t j(1); j <= jend; ++j) {
                lua_pushinteger(L, j);
                lua_gettable(L, -2);
                default_.push_back(to<int>(L));
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            toad.push_back(std::make_pair(offset, default_));
            lua_pop(L, 1);
        }
        return { toad };
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<TriggerOffsetsAndDefaults>");
        throw;
    }
}
template<>
Collision to<Collision>(lua_State *L, int index) {
    try {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a collision must be a table"));
        }
        int const x(readField<int>(L, "x", index));
        int const y(readField<int>(L, "y", index));
        int const xspeed(readField<int>(L, "xspeed", index));
        int const yspeed(readField<int>(L, "yspeed", index));
        int const prevXspeed(readField<int>(L, "prevXspeed", index));
        int const prevYspeed(readField<int>(L, "prevYspeed", index));
        int const width(readField<int>(L, "width", index));
        int const height(readField<int>(L, "height", index));
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection", index));

        return Collision(x, y, xspeed, yspeed, prevXspeed, prevYspeed, width, height, timeDirection);
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Collision>");
        throw;
    }
}
template<>
Glitz to<Glitz>(lua_State *L, int index) {
    try {
        lua_getfield(L, index, "type");
        if (!lua_isstring(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Glitz.type values must be strings"));
        }
        char const * const type(lua_tostring(L, -1));
        if (strcmp(type, "rectangle") == 0) {
            lua_pop(L, 1);
            int const layer(readField<int>(L, "layer", index));
            int const x(readField<int>(L, "x", index));
            int const y(readField<int>(L, "y", index));
            int const width(readField<int>(L, "width", index));
            int const height(readField<int>(L, "height", index));
            unsigned const colour(readColourField(L, "colour"));
            return Glitz(new (multi_thread_tag{}) RectangleGlitz(layer, x, y, width, height, colour));
        }
        else if (strcmp(type, "text") == 0) {
            lua_pop(L, 1);
            int const layer(readField<int>(L, "layer", index));
            auto const text(readField<mt::std::string>(L, "text", index));
            int const x(readField<int>(L, "x", index));
            int const y(readField<int>(L, "y", index));
            int const size(readField<int>(L, "size", index));
            unsigned const colour(readColourField(L, "colour"));
            return Glitz(new (multi_thread_tag{}) TextGlitz(layer, std::move(text), x, y, size, colour));
        }
        else if (strcmp(type, "image") == 0) {
            lua_pop(L, 1);
            int const layer(readField<int>(L, "layer"));
            auto const key(readField<mt::std::string>(L, "key"));
            int const x(readField<int>(L, "x"));
            int const y(readField<int>(L, "y"));
            int const width(readField<int>(L, "width"));
            int const height(readField<int>(L, "height"));

            return Glitz(new (multi_thread_tag{}) ImageGlitz(layer, std::move(key), x, y, width, height));
        }
        std::stringstream ss;
        ss << "Unknown Glitz Type: " << type;
        lua_pop(L, 1);

        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "to<Glitz>");
        throw;
    }
}

unsigned readColourField(lua_State *L, char const *fieldName) {
    try {
        LuaStackManager stack_manager(L);
        lua_getfield(L, -1, fieldName);
        unsigned const r(readField<int>(L, "r"));
        unsigned const g(readField<int>(L, "g"));
        unsigned const b(readField<int>(L, "b"));
        lua_pop(L, 1);
        return r << 24 | g << 16 | b << 8;
    }
    catch (LuaError &e) {
        add_semantic_callstack_info(e, "readColourField");
        throw;
    }
}


} //namespace hg
