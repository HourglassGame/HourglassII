#include "DirectLuaTriggerSystem.h"

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua/lstate.h"

#include "CommonTriggerCode.h"
#include "ObjectAndTime.h"
#include "LuaUtilities.h"
#include "LuaStackManager.h"
#include "LuaSandbox.h"
#include "RectangleGlitz.h"
#include "TextGlitz.h"
#include "ImageGlitz.h"
#include "memory_pool.h"

#include <boost/polymorphic_cast.hpp>
#include <boost/ref.hpp>
#include <gsl/gsl>

#include <iostream>

namespace hg {


//To identify which asserts are actual asserts, and which are checking results from lua
//luaassert is checking results from lua, and should eventually be replaced with an exception

TriggerFrameState DirectLuaTriggerSystem::getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const
{
    LuaState &sharedState(luaStates_->get());
    //Load if not already loaded.
    //Reload if status != LUA_OK to clear possible errors in case of previous TriggerFrameState
    //execution being exited through exception. (Should never happen currently, but maybe will in future)
    if (!sharedState.ptr || lua_status(sharedState.ptr) != LUA_OK) {
        Expects(!sharedState.ptr && "if lua_status != LUA_OK, you are doing something unusual; think about what you are doing and perhaps remove this assert");
        LuaState newLuaState{LuaState::new_state_t()};
        lua_State *L = newLuaState.ptr;

        loadSandboxedLibraries(L);

        pushFunctionFromVector(L, compiledMainChunk_, "triggerSystem");
        setUpPreloadResetFunction(L, compiledExtraChunks_);

        sharedState = std::move(newLuaState);
    }

    return TriggerFrameState(
        new (pool) DirectLuaTriggerFrameState(
            sharedState,
            triggerOffsetsAndDefaults_,
            arrivalLocationsSize_,
            pool,
            interrupter));
}

DirectLuaTriggerFrameState::DirectLuaTriggerFrameState(
    LuaState &sharedState,
    std::vector<
        std::pair<
            int,
            std::vector<int>
        >
    > const &triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize,
    memory_pool<user_allocator_tbb_alloc> &pool,
    OperationInterrupter &interrupter) :
        pool_(pool),
        interrupter_(interrupter),
        L_(sharedState),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize),
        interruptionHandle_(makeInterruptable(L_.ptr, interrupter))
{
    lua_State *L(L_.ptr);
    LuaStackManager stackSaver(L);
    checkstack(L, 1);
    lua_pushvalue(L, -1);
    restoreGlobals(L);
    lua_call(L, 0, 0);
}
namespace {
//Gives the value of the element at the top of the stack of L,
//interpreted as a Box.
Box toBox(lua_State *L, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    if (!lua_istable(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a box must be a table"));
    }
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    int size(readField<int>(L, "size"));
    int illegalPortal(-1);
    lua_getfield(L, -1, "illegalPortal");
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1))
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("illegalPortal must be nil or a positive integer."));
        }
        illegalPortal = lua_index_to_C_index(static_cast<int>(lua_tonumber(L, -1)));
        if (0 > illegalPortal)
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("illegalPortal must be nil or a positive integer."));
        }
    }
    lua_pop(L, 1);
    int arrivalBasis(-1);
    lua_getfield(L, -1, "arrivalBasis");
    if (!lua_isnil(L, -1)) {
        if (!lua_isnumber(L, -1))
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("arrivalBasis must be nil or an integer in the range [1, arrivalLocationsSize+1)"));
        }
        arrivalBasis = lua_index_to_C_index(static_cast<int>(lua_tonumber(L, -1)));
        if (arrivalBasis < 0 || static_cast<int>(arrivalLocationsSize) <= arrivalBasis)
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("arrivalBasis must be nil or an integer in the range [1, arrivalLocationsSize+1)"));
        }
    }
    lua_pop(L, 1);
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    return Box(x, y, xspeed, yspeed, size, illegalPortal, arrivalBasis, timeDirection);
}

mt::std::string abilityToString(Ability ability)
{
    switch (ability) {
    default:
    case Ability::NO_ABILITY:
        Expects(false);
        return mt::std::string("noAbility");
    case Ability::TIME_JUMP:
        return mt::std::string("timeJump");
    case Ability::TIME_REVERSE:
        return mt::std::string("timeReverse");
    case Ability::TIME_GUN:
        return mt::std::string("timeGun");
    case Ability::TIME_PAUSE:
        return mt::std::string("timePause");
    }
}

PortalArea toPortal(lua_State *L, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    if (!lua_istable(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a portal must be a table"));
    }

    int index(lua_index_to_C_index(readField<int>(L, "index")));
    //TODO: better rounding!
    if (index < 0) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("A portal's index must be > 0"));
    }
    
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));

    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    int collisionOverlap(readField<int>(L, "collisionOverlap"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    int destinationIndex(-1);
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "destinationIndex");
    if (!lua_isnil(L, -1)) {
        destinationIndex = lua_index_to_C_index(to<int>(L));
        if (!(destinationIndex >= 0 && static_cast<std::size_t>(destinationIndex) < arrivalLocationsSize))
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("destinationIndex must be nil, or in the range [1, arrivalLocationsSize+1)"));
        }
    }
    lua_pop(L, 1);
    
    int xDestination(readField<int>(L, "xDestination"));
    int yDestination(readField<int>(L, "yDestination"));
    bool relativeTime(readField<bool>(L, "relativeTime"));
    int timeDestination(readField<int>(L, "timeDestination"));
    bool relativeDirection(readField<bool>(L, "relativeDirection"));
    TimeDirection destinationDirection(readField<TimeDirection>(L, "destinationDirection"));
    
    lua_getfield(L, -1, "illegalDestination");
    int illegalDestination;
    if (lua_isnumber(L, -1)) {
        illegalDestination = lua_index_to_C_index(static_cast<int>(std::round(lua_tonumber(L, -1))));
        if (illegalDestination < 0)
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Portal's illegalDestination must be >= 1"));
        }
    }
    else {
        if (!lua_isnil(L, -1))
        {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Portal's illegalDestination must be an integer, or nil."));
        }
        illegalDestination = -1;
    }
    lua_pop(L, 1);
    
    bool fallable(readField<bool>(L, "fallable"));
    bool isLaser(readField<bool>(L, "isLaser"));
    bool winner(readField<bool>(L, "winner"));
    int xaim = 0;
    int yaim = 0;
    int width = 0;
    int height = 0;
    if (isLaser) {
        xaim = readField<int>(L, "xaim");
        yaim = readField<int>(L, "yaim");
    }
    else {
        width = readField<int>(L, "width");
        height = readField<int>(L, "height");
    }
    return
        PortalArea(
            index,
            x, y,
            xaim, yaim,
            width, height,
            xspeed, yspeed,
            collisionOverlap,
            timeDirection, destinationIndex,
            xDestination, yDestination,
            relativeTime,
            timeDestination,
            relativeDirection,
            destinationDirection,
            illegalDestination,
            fallable,
            isLaser,
            winner);
}

MutatorArea toMutatorArea(lua_State *L)
{
    if (!lua_istable(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a mutator must be a table"));
    }
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int width(readField<int>(L, "width"));
    int height(readField<int>(L, "height"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    int collisionOverlap(readField<int>(L, "collisionOverlap"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));

    return
        MutatorArea(
            x, y,
            width, height,
            xspeed, yspeed,
            collisionOverlap,
            timeDirection);
}

ArrivalLocation toArrivalLocation(lua_State *L)
{
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    return ArrivalLocation(x, y, xspeed, yspeed, timeDirection);
}

GlitzPersister toGlitzPersister(lua_State *L) {
    mt::std::string const type(readField<mt::std::string>(L, "type"));
    if (type == "static") {
        Glitz forwardsGlitz = readField<Glitz>(L, "forwardsGlitz");
        Glitz reverseGlitz = readField<Glitz>(L, "reverseGlitz");
        int lifetime = readField<int>(L, "lifetime");
        if (lifetime < 0) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Glitz lifetimes must be positive"));
        }
        TimeDirection timeDirection = readField<TimeDirection>(L, "timeDirection");
        return GlitzPersister(
            mt::std::make_unique<StaticGlitzPersister>(
                std::move(forwardsGlitz), std::move(reverseGlitz), lifetime, timeDirection));
    }
    else if (type == "audio") {
        mt::std::string key = readField<mt::std::string>(L, "key");
        int duration = readField<int>(L, "duration");
        TimeDirection timeDirection = readField<TimeDirection>(L, "timeDirection");
        return GlitzPersister(
            mt::std::make_unique<AudioGlitzPersister>(std::move(key), duration, timeDirection));
    }
    std::stringstream ss;
    ss << "Unknown Glitz Persister Type: " << type;
    BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
}

Box readBoxField(lua_State *L, char const *fieldName, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, fieldName);
    Box retv(toBox(L, arrivalLocationsSize));
    lua_pop(L, 1);
    return retv;
}

ObjectAndTime<Box, Frame *> toObjectAndTimeBox(lua_State *L, Frame *currentFrame, std::size_t arrivalLocationsSize)
{
    return ObjectAndTime<Box, Frame *>(
        readBoxField(L, "box", arrivalLocationsSize),
        getArbitraryFrame(getUniverse(currentFrame), readField<int>(L, "targetFrame")));
}

}

PhysicsAffectingStuff
DirectLuaTriggerFrameState::calculatePhysicsAffectingStuff(
    Frame const *currentFrame,
    boost::transformed_range<
    GetBase<TriggerDataConstPtr>,
    mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) try {
    //All indicies viewed or written by lua count starting from 1, and are adjusted
    //before geing used in c++.
    //Expects a chunk which, when executed, returns 1 element:
    //a table with a member function
    //called "calculatePhysicsAffectingStuff", which takes
    //two arguments, 1st) a `self` paramater which is that table itself
    //and 2nd) a table containing the apparent trigger arrivals, stored
    //as a array-table of array-tables, where the index in the outer array gives the
    //trigger ID, and the inner array gives the value for that trigger.
    //This function must return one value: a table; which may optionally
    //contain any of the following:
    //An array-table called "additionalBoxes", containing
    //tables with the following format:
    /*
        {
            x = <nuumber>,
            y = <number>,
            xspeed = <number>,
            yspeed = <number>,
            size = <number>,
            illegalPortal = <nil or positive number>,
            arrivalBasis = <nil or number in the range [1, arrivalLocationsSize]>,
            timeDirection = <'forwards' or 'reverse'>
        }
    */
    //An array-table called "collisions", containing
    //tables with the following format:
    /*
        {
            x = <number>,
            y = <number>,
            xspeed = <number>,
            yspeed = <number>,
            width = <number>,
            height = <number>,
            timeDirection = <'forwards' or 'reverse'>
        }
    */
    //An array-table called "portals", containing
    //tables with the following format:
    /*
        {
            --index is used for identifying a portal for the purposes of
            --determining which portals are illegal
            --(in this context `illegal` means that an object may not fall through
            --the portal)
            index = <positive number>,
            x = <number>,
            y = <number>,
            width = <positive number or 0>,
            height = <positive number or 0>,
            xspeed = <number>,
            yspeed = <number>,
            collisionOverlap = <number in range [0, 100]>,
            timeDirection = <'forwards' or 'reverse'>,
            destinationIndex = <number between 1 and arrivalLocationsSize inclusive or nil>,
            xDestination = <number>,
            yDestination = <number>,
            relativeTime = <boolean>,
            timeDestination = <number>,
            relativeDirection = <boolean>,
            destinationDirection = <'forwards' or 'reverse'>,
            --nil indicates that no portals should be illegal
            --for an object upon the arrival of that object
            --at its destination after travelling though this portal
            illegalDestination = <positive number or nil>,
            fallable = <boolean>,
            winner = <boolean>
        }
    */
    /*
    //An array-table called "mutators", containing
    //tables with the following format
        {
            x = <number>,
            y = <number>,
            width = <positive number or 0>,
            height = <positive number or 0>,
            xspeed = <number>,
            yspeed = <number>,
            collisionOverlap = <number in range [0, 100]>,
            timeDirection = <'forwards' or 'reverse'>
        }
    */
    //An array-table called "arrivalLocations", containing
    //tables with the following format
    /*
        {
            x = <number>,
            y = <number>,
            xspeed = <number>,
            yspeed = <number>,
            timeDirection = <'forwards' or 'reverse'>
        }
    */
    //IMPORTANT! - The "arrivalLocations" array-table is compulsary,
    //and must always contain the same number of tables.
    //Furthermore, the "destinationIndex" field of those tables
    //that need it must contain an integer in the range [1, n]
    //where n is that number of tables in arrivalLocations.

    lua_State *L(L_.ptr);

    mp::std::vector<mp::std::vector<int>>
        apparentTriggers(calculateApparentTriggers(triggerOffsetsAndDefaults_, triggerArrivals, pool_));

    PhysicsAffectingStuff retv(pool_);

    LuaStackManager stackSaver(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "calculatePhysicsAffectingStuff");
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("calculatePhysicsAffectingStuff must be a function"));
    }
    //push `frameNumber` argument
    luaL_checkstack(L, 1, nullptr);
    lua_pushinteger(L, getFrameNumber(currentFrame));
    //push `triggerArrivals` argument [
    luaL_checkstack(L, 1, nullptr);
    lua_createtable(L, static_cast<int>(boost::size(triggerArrivals)), 0);
    //create index and table for each trigger
    {
        int i(0);
        for (mp::std::vector<int> const &apparentTrigger : apparentTriggers) {
            ++i;
            luaL_checkstack(L, 1, nullptr);
            lua_createtable(L, static_cast<int>(apparentTrigger.size()), 0);
            //insert each triggerElement into the table for the particular trigger
            int j(0);
            for (int triggerElement : apparentTrigger) {
                ++j;
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, triggerElement);
                lua_rawseti(L, -2, j);
            }
            lua_rawseti(L, -2, i);
        }
    }
    //]
    //call function

    lua_call(L, 2, 1);

    //read return value
    //TODO: better handling of sparsely populated tables.
    
    //read 'additionalBoxes' table:
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "additionalBoxes");
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("additionalBoxes must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.additionalBoxes.push_back(toBox(L, static_cast<int>(arrivalLocationsSize_)));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'collisions' table:
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "collisions");
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            throw("collisions must be a table");
        }
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.collisions.push_back(to<Collision>(L));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'portals' table
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "portals");
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("portals must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.portals.push_back(toPortal(L, arrivalLocationsSize_));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'mutators' table
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "mutators");
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("mutators must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.mutators.push_back(toMutatorArea(L));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'arrivalLocations' table
    luaL_checkstack(L, 1, nullptr);
    lua_getfield(L, -1, "arrivalLocations");
    if (!lua_istable(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("arrivalLocations must be a table"));
    }
    for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
        if (end != arrivalLocationsSize_) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("arrivalLocations must contain 'arrivalLocationsSize' elements"));
        }
        luaL_checkstack(L, 1, nullptr);
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        retv.arrivalLocations.push_back(toArrivalLocation(L));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    
    //pop return value
    lua_pop(L, 1);
    return retv;
}
catch (LuaError &e) {
    add_semantic_callstack_info(e, "calculatePhysicsAffectingStuff");
    throw;
}


namespace {
void pushGuy(lua_State *L, Guy const &guy)
{
    luaL_checkstack(L, 1, nullptr);
    lua_createtable(L, 0, 17);
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, nullptr);
    lua_pushinteger(L, guy.getIndex());
    lua_setfield(L, -2, "index");
    lua_pushinteger(L, guy.getX());
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, guy.getY());
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, guy.getXspeed());
    lua_setfield(L, -2, "xspeed");
    lua_pushinteger(L, guy.getYspeed());
    lua_setfield(L, -2, "yspeed");
    lua_pushinteger(L, guy.getWidth());
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, guy.getHeight());
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, guy.getJumpSpeed());
    lua_setfield(L, -2, "jumpSpeed");
    if (guy.getIllegalPortal() != -1) {
        lua_pushinteger(L, guy.getIllegalPortal() + 1);
        lua_setfield(L, -2, "illegalPortal");
    }
    if (guy.getArrivalBasis() != -1) {
        lua_pushinteger(L, guy.getArrivalBasis() + 1);
        lua_setfield(L, -2, "arrivalBasis");
    }
    lua_pushboolean(L, guy.getSupported());
    lua_setfield(L, -2, "supported");
    if (guy.getSupported()) {
        lua_pushinteger(L, guy.getSupportedSpeed());
        lua_setfield(L, -2, "supportedSpeed");
    }
    
    lua_createtable(L, static_cast<int>(guy.getPickups().size()), 0);
    typedef std::pair<Ability const, int> PickupPair;
    for (PickupPair const &pickup: guy.getPickups()) {
        luaL_checkstack(L, 2, nullptr);
        lua_pushstring(L, abilityToString(pickup.first).c_str());
        lua_pushinteger(L, pickup.second);
        lua_rawset(L, -3);
    }
    lua_setfield(L, -2, "pickups");
    
    lua_pushstring(L, guy.getFacing() == FacingDirection::LEFT ? "left" : "right");
    lua_setfield(L, -2, "facing");
    
    lua_pushboolean(L, guy.getBoxCarrying());
    lua_setfield(L, -2, "boxCarrying");
    if (guy.getBoxCarrying()) {
        lua_pushinteger(L, guy.getBoxCarrySize());
        lua_setfield(L, -2, "boxCarrySize");
        lua_pushstring(L, guy.getBoxCarryDirection() == TimeDirection::FORWARDS ? "forwards" : "reverse");
        lua_setfield(L, -2, "boxCarryDirection");
    }
    lua_pushstring(L, guy.getTimeDirection() == TimeDirection::FORWARDS ? "forwards" : "reverse");
    lua_setfield(L, -2, "timeDirection");
    lua_pushboolean(L, guy.getTimePaused());
    lua_setfield(L, -2, "timePaused");
}

void pushBox(lua_State *L, Box const &box)
{
    luaL_checkstack(L, 1, nullptr);
    lua_createtable(L, 0, 10);
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, nullptr);
    lua_pushinteger(L, box.getX());
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, box.getY());
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, box.getXspeed());
    lua_setfield(L, -2, "xspeed");
    lua_pushinteger(L, box.getYspeed());
    lua_setfield(L, -2, "yspeed");
//TODO: better solution for the need(?)
//for redundant width and height fields
//in boxes that arises from using them 
//in the same context as guys.
    lua_pushinteger(L, box.getSize());
    lua_setfield(L, -2, "size");
    lua_pushinteger(L, box.getWidth());
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, box.getHeight());
    lua_setfield(L, -2, "height");
    if (box.getIllegalPortal() != -1) {
        lua_pushinteger(L, box.getIllegalPortal() + 1);
        lua_setfield(L, -2, "illegalPortal");
    }
    if (box.getArrivalBasis() != -1) {
        lua_pushinteger(L, box.getArrivalBasis() + 1);
        lua_setfield(L, -2, "arrivalBasis");
    }
    lua_pushstring(L, box.getTimeDirection() == TimeDirection::FORWARDS ? "forwards" : "reverse");
    lua_setfield(L, -2, "timeDirection");
}
}

//TODO: fix code duplication with box version and portal version
//ARGH so much code duplication ):
bool DirectLuaTriggerFrameState::shouldArrive(Guy const &potentialArriver)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "shouldArrive");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldArrive must be a function"));
    }
    //push `potentialArriver` argument
    pushGuy(L, potentialArriver);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 1, 1);
    //read return value
    if (!lua_isboolean(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldArrive must return a boolean value"));
    }
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool DirectLuaTriggerFrameState::shouldArrive(Box const &potentialArriver)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "shouldArrive");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldArrive must be a function"));
    }
    //push `potentialArriver` argument
    pushBox(L, potentialArriver);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 1, 1);
    //read return value
    if (!lua_isboolean(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldArrive must return a boolean value"));
    }
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}

//JUST TO BE PERFECTLY CLEAR:
//`responsiblePortalIndex` does not refer to 
//`portal.getIndex()` (which is just for illegal portals)
//it refers to the `i` in `nextPortal[i]`.
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Guy const &potentialPorter,
    bool porterActionedPortal)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "shouldPort");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldPort must be a function"));
    }
    //push `responsiblePortalIndex` argument
    luaL_checkstack(L, 1, nullptr);
    lua_pushinteger(L, responsiblePortalIndex + 1);
    //push `potentialPorter` argument
    pushGuy(L, potentialPorter);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //push `porterActionedPortal` argument
    luaL_checkstack(L, 1, nullptr);
    lua_pushboolean(L, porterActionedPortal);
    //call function
    lua_call(L, 3, 1);
    //read return value
    if (!lua_isboolean(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldPort must return a boolean value"));
    }
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Box const &potentialPorter,
    bool porterActionedPortal)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "shouldPort");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldPort must be a function"));
    }
    //push `responsiblePortalIndex` argument
    luaL_checkstack(L, 1, nullptr);
    lua_pushinteger(L, responsiblePortalIndex + 1);
    //push `potentialPorter` argument
    pushBox(L, potentialPorter);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //push `porterActionedPortal` argument
    luaL_checkstack(L, 1, nullptr);
    lua_pushboolean(L, porterActionedPortal);
    //call function
    lua_call(L, 3, 1);
    //read return value
    if (!lua_isboolean(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("shouldPort must return a boolean value"));
    }
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}


//Unfortunately the current implementation allows lua to return all sorts of nonsensical things
//for Guys (eg, change relative index, change illegalPortal, supportedSpeed etc.. none of these make much sense)
boost::optional<Guy> DirectLuaTriggerFrameState::mutateObject(
    mp::std::vector<int> const &responsibleMutatorIndices,
    Guy const &objectToManipulate)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "mutateObject");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("mutateObject must be a function"));
    }
    //push responsibleMutatorIndices argument
    luaL_checkstack(L, 1, nullptr);
    lua_createtable(L, static_cast<int>(responsibleMutatorIndices.size()), 0);
    //insert each triggerElement into the table for the particular trigger
    int i(0);
    for (int const mutatorIndex: responsibleMutatorIndices) {
        ++i;
        luaL_checkstack(L, 1, nullptr);
        lua_pushinteger(L, C_index_to_lua_index(mutatorIndex));
        lua_rawseti(L, -2, i);
    }
    //push dynamicObject argument
    pushGuy(L, objectToManipulate);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    boost::optional<Guy> retv;
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("mutated object must be a table"));
        }
        retv = to<Guy>(L);
    }
    //pop return value
    lua_pop(L, 1);
    return retv;
}
boost::optional<Box> DirectLuaTriggerFrameState::mutateObject(
    mp::std::vector<int> const &responsibleMutatorIndices,
    Box const &objectToManipulate)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "mutateObject");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("mutateObject must be a function"));
    }
    //push responsibleMutatorIndices argument
    luaL_checkstack(L, 1, nullptr);
    lua_createtable(L, static_cast<int>(responsibleMutatorIndices.size()), 0);
    //insert each triggerElement into the table for the particular trigger
    int i(0);
    for (int const mutatorIndex: responsibleMutatorIndices) {
        ++i;
        luaL_checkstack(L, 1, nullptr);
        lua_pushinteger(L, C_index_to_lua_index(mutatorIndex));
        lua_rawseti(L, -2, i);
    }
    //push dynamicObject argument
    pushBox(L, objectToManipulate);
    luaL_checkstack(L, 1, nullptr);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    boost::optional<Box> retv;
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("mutated object must be a table"));
        }
        retv = toBox(L, static_cast<int>(arrivalLocationsSize_));
    }
    //pop return value
    lua_pop(L, 1);
    return retv;
}

TriggerFrameStateImplementation::DepartureInformation DirectLuaTriggerFrameState::getDepartureInformation(
    mt::std::map<Frame *, ObjectList<Normal>> const &departures,
    Frame *currentFrame)
{
    lua_State *L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, nullptr);
    lua_getglobal(L, "getDepartureInformation");
    if (!lua_isfunction(L, -1)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("getDepartureInformation must be a table"));
    }
    //push `departures` argument [
    luaL_checkstack(L, 1, nullptr);
    //TODO find out if sparse arrays count as
    //array elements or non-array elements
    //TODO: Finesse this table sizing for the case where departures contains a departure to the nullFrame.
    lua_createtable(L, static_cast<int>(departures.size()), 0);
    for (std::pair<Frame *, ObjectList<Normal>> const &departureSection : departures)
    {
        luaL_checkstack(L, 1, nullptr);
        lua_createtable(L, 0, 2);
        {
            luaL_checkstack(L, 1, nullptr);
            lua_createtable(L, static_cast<int>(departureSection.second.getList<Guy>().size()), 0);
            int j(0);
            for (Guy const &guy : departureSection.second.getList<Guy>()) {
                ++j;
                pushGuy(L, guy);
                lua_rawseti(L, -2, j);
            }
            lua_setfield(L, -2, "guys");
        }
        {
            luaL_checkstack(L, 1, nullptr);
            lua_createtable(L, static_cast<int>(departureSection.second.getList<Box>().size()), 0);
            int j(0);
            for (Box const &box : departureSection.second.getList<Box>()) {
                ++j;
                pushBox(L, box);
                lua_rawseti(L, -2, j);
            }
            lua_setfield(L, -2, "boxes");
        }
        //Departure to null frame represented by -1. Is this good? Should a non-numerical key be used instead?
        lua_rawseti(L, -2, isNullFrame(departureSection.first) ? -1 : getFrameNumber(departureSection.first)+1);
    }
    //]
    //call function
    //return values are: triggers, forwardsGlitz, reverseGlitz, glitzPersisters, extraBoxes
    lua_call(L, 1, 5);
    
    //[triggers,forwardsGlitz,reverseGlitz,glitzPersisters,extraBoxes]

    //read triggers return value
    //Trigger return value looks like:
    /*
    {
        [1] = {13},
        [3] = {100, 1020}, --Note that trigger return values may be sparse/incomplete. Missing values take their default values (when they arrive).
        [4] = {},
        [5] = {2131, 144, 70, 154}
        [trigger index] = {trigger value},
        ...
    }
    */
    mp::std::vector<TriggerData> triggers(pool_);
    if (!lua_istable(L, -5)) {
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Trigger List must be a table"));
    }
    luaL_checkstack(L, 2, nullptr);
    {
        lua_pushnil(L); //[triggers,forwardsGlitz,reverseGlitz,glitzPersisters,extraBoxes,nil]
        while (lua_next(L, -6) != 0) {
            //[triggers,forwardsGlitz,reverseGlitz,glitzPersisters,extraBoxes,triggerIndex,triggerValue]
            if (!lua_isnumber(L, -2)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Trigger List must contain only integer keys"));
            }
            int index(lua_index_to_C_index(static_cast<int>(lua_tonumber(L, -2))));
            mt::std::vector<int> value;
            if (!lua_istable(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("trigger value must be a table"));
            }
            for (std::size_t k(1), end(lua_rawlen(L, -1)); k <= end; ++k) {
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, k);
                lua_gettable(L, -2);
                if (!lua_isnumber(L, -1)) {
                    BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("trigger data elements must be integers"));
                }
                value.push_back(static_cast<int>(lua_tonumber(L, -1)));
                lua_pop(L, 1);
            }
            triggers.push_back(TriggerData(index, value));
            lua_pop(L, 1);
        }
        if (boost::size(triggers) > boost::size(triggerOffsetsAndDefaults_)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("The trigger system lua must not create more trigger departures than those declared with offsets and defaults"));
        }
    }
    //read glitz return values
    //Glitz return value looks like this:
    /*
    {
        {
            type = <'rectangle' or 'text'>
            if type == 'rectangle'
              layer = <number>,
              x = <number>,
              y = <number>,
              width = <number>
              height = <number>,
              colour = {r = <number>, g = <number>, b = <number>},
            elseif type == 'text'
              layer = <number>,
              text = <string>,
              x = <number>,
              y = <number>,
              size = <number>,
              colour = {r = <number>, g = <number>, b = <number>},
            elseif type == 'image'
              layer = <number>,
              key = <string>,
              x = <number>
              y = <number>,
              width = <number>,
              height = <number>
            end
        },
        {
            <as above>
        },
        ...
    }*/

    // get forwards glitz departure
    mt::std::vector<Glitz> forwardsGlitz;
    if (!lua_isnil(L, -4)) {
        if (!lua_istable(L, -4)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("forwards glitz list must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -4)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -5);
            forwardsGlitz.push_back(to<Glitz>(L));
            lua_pop(L, 1);
        }
    }
    
    // get reverse glitz departure
    mt::std::vector<Glitz> reverseGlitz;
    if (!lua_isnil(L, -3)) {
        if (!lua_istable(L, -3)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("background glitz list must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -3)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -4);
            reverseGlitz.push_back(to<Glitz>(L));
            lua_pop(L, 1);
        }
    }

    
    //Get GlitzPersisters departure
    //glitz persister return value looks like:
    /*
    {
        {
            type = <'static' or 'audio'>,
            if type == 'static' then
                forwardsGlitz = <Glitz (as above)>,
                reverseGlitz = <Glitz (as above)>,
                lifetime = <natural number>,
                timeDirection = <'forwards' or 'reverse'>
            elseif type == 'audio' then
                key = <string>,
                duration = <natural number>,
                timeDirection = <'forwards' or 'reverse'>
            end
        },
        ...
    }
    */
    mt::std::vector<GlitzPersister> glitzPersisters;
    if (!lua_isnil(L, -2)) {
        if (!lua_istable(L, -2)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("glitz persisters list must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -2)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -3);
            glitzPersisters.push_back(toGlitzPersister(L));
            lua_pop(L, 1);
        }
    }

    // get extra boxes to depart
    // Extra boxes look like this:
    /*
    {
        {
            box =
            --should be same format as boxes in additionalBoxes
            {
                x = <nuumber>,
                y = <number>,
                xspeed = <number>,
                yspeed = <number>,
                size = <number>,
                illegalPortal = <nil or positive number>,
                arrivalBasis = <nil or number in the range [1, arrivalLocationsSize]>
                timeDirection = <'forwards' or 'reverse'>
            }
            targetFrame = <number in range [0, timelineLength)>

        },
        {
            <as above>
        },
        ...
    }
    */
    mt::std::vector<ObjectAndTime<Box, Frame *>> newBox;
    if (!lua_isnil(L, -1)) {
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("extra boxes list must be a table"));
        }
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, nullptr);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            newBox.push_back(toObjectAndTimeBox(L, currentFrame, arrivalLocationsSize_));
            lua_pop(L, 1);
        }
    }
    
    //pop return values
    lua_pop(L, 5);
    return {
        calculateActualTriggerDepartures(triggers, triggerOffsetsAndDefaults_, currentFrame),
        forwardsGlitz,
        reverseGlitz,
        glitzPersisters,
        newBox};
}

DirectLuaTriggerFrameState::~DirectLuaTriggerFrameState() noexcept
{
}

namespace {
std::vector<char> compileLuaChunk(std::vector<char> const &sourceChunk, char const *name) {
    std::pair<char const *, char const *> source_iterators;
    if (!sourceChunk.empty()) {
        source_iterators.first = &sourceChunk.front();
        source_iterators.second = &sourceChunk.front() + sourceChunk.size();
    }
    std::vector<char> compiledChunk;
    LuaState L{LuaState::new_state_t{}};
    if (lua_load(L.ptr, lua_VectorReader, &source_iterators, name, nullptr)) {
        std::stringstream ss;
        ss << "Couldn't compile lua chunk: " << name << ", got error: " << lua_tostring(L.ptr, -1);
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
    }
    if (lua_dump(L.ptr, lua_VectorWriter, &compiledChunk, true)) {
        Expects(false && "Failure to insert into the vector should have resulted in bad_alloc being thrown, rather than a non-zero return value from lua_VectorWriter");
    }
    return compiledChunk;
}
}
DirectLuaTriggerSystem::DirectLuaTriggerSystem(
    std::vector<char> const &mainChunk,
    std::vector<LuaModule> const &extraChunks,
    std::vector<
            std::pair<
                int,
                std::vector<int>
            >
    > triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize) :
        compiledMainChunk_(compileLuaChunk(mainChunk, "Main Chunk")),
        compiledExtraChunks_(extraChunks),
        triggerOffsetsAndDefaults_(std::move(triggerOffsetsAndDefaults)),
        arrivalLocationsSize_(arrivalLocationsSize)
{
    for (LuaModule &module: compiledExtraChunks_) {
        module.chunk = compileLuaChunk(module.chunk, module.name.c_str());
    }
}
}

