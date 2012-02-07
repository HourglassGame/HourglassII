#include "DirectLuaTriggerSystem.h"
#include <cassert>
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "CommonTriggerCode.h"
#include "ObjectAndTime.h"
#include "LuaUtilities.h"
#include "Foreach.h"
#include "LuaStackManager.h"
#include <boost/ref.hpp>
#include <iostream>
namespace hg {
//To identify which asserts are actual asserts, and which are checking results from lua
#define luaassert assert
TriggerFrameState DirectLuaTriggerSystem::getFrameState(OperationInterrupter& interrupter) const
{
    LuaState& sharedState(luaStates_->get());

    if (!sharedState.ptr) {
        sharedState = loadLuaStateFromVector(compiledLuaChunk_, "triggerSystem");
        loadSandboxedLibraries(sharedState.ptr);
    }

	return TriggerFrameState(
		multi_thread_new<DirectLuaTriggerFrameState>(
			boost::ref(sharedState),
			boost::cref(triggerOffsetsAndDefaults_),
			arrivalLocationsSize_,
            boost::ref(interrupter)));
}

DirectLuaTriggerFrameState::DirectLuaTriggerFrameState(
    LuaState& sharedState,
    std::vector<
        std::pair<
            int,
            std::vector<int>
        >
    > const& triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize,
    OperationInterrupter& interrupter) :
        L_(sharedState),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize)
{
    lua_State* L(L_.ptr);
    LuaStackManager stackSaver(L);
    LuaInterruptionHandle h(makeInterruptable(L, interrupter));
    luaL_checkstack(L, 1, 0);
    lua_pushvalue(L, -1);
    sandboxFunction(L, -1);
    lua_call(L, 0, 0);
}
namespace {
//Gives the value of the element at the top of the stack of L,
//interpreted as a Box.
Box toBox(lua_State* L, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    luaassert(lua_istable(L, -1) && "a box must be a table");
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    int size(readField<int>(L, "size"));
    int illegalPortal(-1);
    lua_getfield(L, -1, "illegalPortal");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_isnumber(L, -1));
        illegalPortal = static_cast<int>(lua_tonumber(L, -1)) - 1;
        luaassert(0 <= illegalPortal);
    }
    lua_pop(L, 1);
    int arrivalBasis(-1);
    lua_getfield(L, -1, "arrivalBasis");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_isnumber(L, -1));
        arrivalBasis = static_cast<int>(lua_tonumber(L, -1)) - 1;
        luaassert(0 <= arrivalBasis && arrivalBasis < arrivalLocationsSize);
    }
    lua_pop(L, 1);
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    return Box(x, y, xspeed, yspeed, size, illegalPortal, arrivalBasis, timeDirection);
}

std::string abilityToString(Ability ability)
{
    switch (ability) {
    case NO_ABILITY:
        assert(false);
        return std::string("noAbility");
    case TIME_JUMP:
        return std::string("timeJump");
    case TIME_REVERSE:
        return std::string("timeReverse");
    case TIME_GUN:
        return std::string("timeGun");
    default:
        assert(false);
        return std::string("fix abilityToString(Ability ability) !!");
    }
}

PortalArea toPortal(lua_State* L, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    luaassert(lua_istable(L, -1) && "a portal must be a table");
    
    int index(readField<int>(L, "index") - 1);
    //TODO: better rounding!
    luaassert(index >= 0);
    
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int width(readField<int>(L, "width"));
    int height(readField<int>(L, "height"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    int collisionOverlap(readField<int>(L, "collisionOverlap"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    int destinationIndex(-1);
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "destinationIndex");
    if (!lua_isnil(L, -1)) {
        destinationIndex = to<int>(L) - 1;
        luaassert(destinationIndex >= 0 && static_cast<std::size_t>(destinationIndex) < arrivalLocationsSize);
    }
    lua_pop(L, 1);
    
    int xDestination(readField<int>(L, "xDestination"));
    int yDestination(readField<int>(L, "yDestination"));
    bool relativeTime(readField<bool>(L, "relativeTime"));
    int timeDestination(readField<int>(L, "timeDestination"));
    
    lua_getfield(L, -1, "illegalDestination");
    int illegalDestination;
    if (lua_isnumber(L, -1)) {
        illegalDestination = static_cast<int>(lua_tointeger(L, -1)) - 1;
        luaassert(illegalDestination >= 0);
    }
    else {
        luaassert(lua_isnil(L, -1));
        illegalDestination = -1;
    }
    lua_pop(L, 1);
    
    bool fallable(readField<bool>(L, "fallable"));
    bool winner(readField<bool>(L, "winner"));
    
    return
        PortalArea(
            index,
            x, y,
            width, height,
            xspeed, yspeed,
            collisionOverlap,
            timeDirection, destinationIndex,
            xDestination, yDestination,
            relativeTime,
            timeDestination,
            illegalDestination,
            fallable,
            winner);
}

MutatorArea toMutatorArea(lua_State* L)
{
    luaassert(lua_istable(L, -1) && "a mutator must be a table");
    
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

ArrivalLocation toArrivalLocation(lua_State* L)
{
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    
    return ArrivalLocation(x, y, xspeed, yspeed, timeDirection);
}

unsigned readColourField(lua_State* L, char const* fieldName)
{
    LuaStackManager stack_manager(L);
    lua_getfield(L, -1, fieldName);
    unsigned r(readField<int>(L, "r"));
    unsigned g(readField<int>(L, "g"));
    unsigned b(readField<int>(L, "b"));
    lua_pop(L, 1);
    return r << 24 | g << 16 | b << 8;
}

RetardedNotActuallyAGlitzGlitz toGlitz(lua_State* L)
{
    int x(readField<int>(L, "x"));
    int y(readField<int>(L, "y"));
    int width(readField<int>(L, "width"));
    int height(readField<int>(L, "height"));
    int xspeed(readField<int>(L, "xspeed"));
    int yspeed(readField<int>(L, "yspeed"));
    unsigned forwardsColour(readColourField(L, "forwardsColour"));
    unsigned reverseColour(readColourField(L, "reverseColour"));
    TimeDirection timeDirection(readField<TimeDirection>(L, "timeDirection"));
    return RetardedNotActuallyAGlitzGlitz(x, y, width, height, xspeed, yspeed, forwardsColour, reverseColour, timeDirection);
}

Box readBoxField(lua_State* L, char const* fieldName, std::size_t arrivalLocationsSize)
{
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, fieldName);
    Box retv(toBox(L, arrivalLocationsSize));
    lua_pop(L, 1);
    return retv;
}

ObjectAndTime<Box, Frame*> toObjectAndTimeBox(lua_State* L, Frame* currentFrame, std::size_t arrivalLocationsSize)
{
    return ObjectAndTime<Box, Frame*>(
        readBoxField(L, "box", arrivalLocationsSize),
        getArbitraryFrame(getUniverse(currentFrame), readField<int>(L, "targetFrame")));
}

}

PhysicsAffectingStuff
    DirectLuaTriggerFrameState::calculatePhysicsAffectingStuff(
        Frame const* currentFrame,
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr>::type const> const& triggerArrivals)
{
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
            arrivalBasis = <nil or number in the range [1, arrivalLocationsSize]>
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
            timeDirection = <'forwards' or 'backwards'>
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
            index = <positive number>
            x = <number>,
            y = <number>,
            width = <positive number or 0>,
            height = <positive number or 0>,
            xspeed = <number>,
            yspeed = <number>,
            collisionOverlap = <number in range [0, 100]>,
            timeDirection = <'forwards' or 'backwards'>,
            destinationIndex = <number between 1 and arrivalLocationsSize inclusive or nil>,
            xDestination = <number>,
            yDestination = <number>,
            relativeTime = <boolean>,
            timeDestination = <number>,
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
            x = <number>
            y = <number>
            width = <positive number or 0>
            height = <positive number or 0>
            xspeed = <number>
            yspeed = <number>
            collisionOverlap = <number in range [0, 100]>
            timeDirection = <'forwards' or 'backwards'>
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
            timeDirection = <'forwards' or 'backwards'>
        }
    */
    //IMPORTANT! - The "arrivalLocations" array-table is compulsary,
    //and must always contain the same number of tables.
    //Furthermore, the "destinationIndex" field of those tables
    //that need it must contain an integer in the range [1, n]
    //where n is that number of tables in arrivalLocations.
    
    lua_State* L(L_.ptr);
    
    mt::std::vector<mt::std::vector<int>::type>::type
        apparentTriggers(calculateApparentTriggers(triggerOffsetsAndDefaults_, triggerArrivals));
        
    PhysicsAffectingStuff retv;

    LuaStackManager stackSaver(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "calculatePhysicsAffectingStuff");
    luaassert(lua_type(L, -1) == LUA_TFUNCTION);
    //push `frameNumber` argument
    luaL_checkstack(L, 1, 0);
    lua_pushinteger(L, getFrameNumber(currentFrame));
    //push `triggerArrivals` argument [
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, static_cast<int>(boost::distance(triggerArrivals)), 0);
    //create index and table for each trigger
    int i(0);
    foreach (mt::std::vector<int>::type const& apparentTrigger, apparentTriggers) {
        ++i;
        luaL_checkstack(L, 1, 0);
        lua_createtable(L, static_cast<int>(apparentTrigger.size()), 0);
        //insert each triggerElement into the table for the particular trigger
        int j(0);
        foreach (int triggerElement, apparentTrigger) {
            ++j;
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, triggerElement);
            lua_rawseti(L, -2, j);
        }
        lua_rawseti(L, -2, i);
    }
    //]
    //call function

    lua_call(L, 2, 1);

    //read return value
    //TODO: better handling of sparsely populated tables.
    
    //read 'additionalBoxes' table:
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "additionalBoxes");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_istable(L, -1) && "additionalBoxes must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.additionalBoxes.push_back(toBox(L, static_cast<int>(arrivalLocationsSize_)));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'collisions' table:
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "collisions");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_istable(L, -1) && "collisions must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.collisions.push_back(to<Collision>(L));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'portals' table
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "portals");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_istable(L, -1) && "portals must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.portals.push_back(toPortal(L, arrivalLocationsSize_));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'mutators' table
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "mutators");
    if (!lua_isnil(L, -1)) {
        luaassert(lua_istable(L, -1) && "mutators must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            retv.mutators.push_back(toMutatorArea(L));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    
    //read 'arrivalLocations' table
    luaL_checkstack(L, 1, 0);
    lua_getfield(L, -1, "arrivalLocations");
    luaassert(lua_istable(L, -1) && "arrivalLocations must be a table");
    for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
        luaassert(end == arrivalLocationsSize_);
        luaL_checkstack(L, 1, 0);
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


namespace {
void pushGuy(lua_State* L, Guy const& guy)
{
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, 0, 17);
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, 0);
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
    foreach (PickupPair const& pickup, guy.getPickups()) {
        luaL_checkstack(L, 2, 0);
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
        lua_pushstring(L, guy.getBoxCarryDirection() == FORWARDS ? "forwards" : "reverse");
        lua_setfield(L, -2, "boxCarryDirection");
    }
    lua_pushstring(L, guy.getTimeDirection() == FORWARDS ? "forwards" : "reverse");
    lua_setfield(L, -2, "timeDirection");
}

void pushBox(lua_State* L, Box const& box)
{
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, 0, 10);
    LuaStackManager stack_manager(L);
    luaL_checkstack(L, 1, 0);
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
    lua_pushstring(L, box.getTimeDirection() == FORWARDS ? "forwards" : "reverse");
    lua_setfield(L, -2, "timeDirection");
}

//TODO: fix code duplication with box version and portal version
bool doShouldXFunction(lua_State* L, char const* functionName, int responsibleXIndex, Guy const& potentialXer)
{
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, functionName);
    luaassert(lua_isfunction(L, -1));
    //push `responsibleXIndex` argument
    luaL_checkstack(L, 1, 0);
    lua_pushinteger(L, responsibleXIndex + 1);
    //push `potentialX` argument
    pushGuy(L, potentialXer);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool doShouldXFunction(lua_State* L, char const* functionName, int responsibleXIndex, Box const& potentialXer)
{
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, functionName);
    luaassert(lua_isfunction(L, -1));
    //push `responsibleXIndex` argument
    luaL_checkstack(L, 1, 0);
    lua_pushinteger(L, responsibleXIndex + 1);
    //push `potentialX` argument
    pushBox(L, potentialXer);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
}


//ARGH so much code duplication ):
bool DirectLuaTriggerFrameState::shouldArrive(Guy const& potentialArriver)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "shouldArrive");
    luaassert(lua_isfunction(L, -1));
    //push `potentialArriver` argument
    pushGuy(L, potentialArriver);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 1, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool DirectLuaTriggerFrameState::shouldArrive(Box const& potentialArriver)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "shouldArrive");
    luaassert(lua_isfunction(L, -1));
    //push `potentialArriver` argument
    pushBox(L, potentialArriver);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 1, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
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
    Guy const& potentialPorter,
    bool porterActionedPortal)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "shouldPort");
    luaassert(lua_isfunction(L, -1));
    //push `responsiblePortalIndex` argument
    luaL_checkstack(L, 1, 0);
    lua_pushinteger(L, responsiblePortalIndex + 1);
    //push `potentialPorter` argument
    pushGuy(L, potentialPorter);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //push `porterActionedPortal` argument
    luaL_checkstack(L, 1, 0);
    lua_pushboolean(L, porterActionedPortal);
    //call function
    lua_call(L, 3, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Box const& potentialPorter,
    bool porterActionedPortal)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "shouldPort");
    luaassert(lua_isfunction(L, -1));
    //push `responsiblePortalIndex` argument
    luaL_checkstack(L, 1, 0);
    lua_pushinteger(L, responsiblePortalIndex + 1);
    //push `potentialPorter` argument
    pushBox(L, potentialPorter);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //push `porterActionedPortal` argument
    luaL_checkstack(L, 1, 0);
    lua_pushboolean(L, porterActionedPortal);
    //call function
    lua_call(L, 3, 1);
    //read return value
    luaassert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}


//Unfortunately the current implementation allows lua to return all sorts of nonsensical things
//for Guys (eg, change relative index, change illegalPortal, supportedSpeed etc.. none of these make much sense)
boost::optional<Guy> DirectLuaTriggerFrameState::mutateObject(
    mt::std::vector<int>::type const& responsibleMutatorIndices,
    Guy const& objectToManipulate)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "mutateObject");
    luaassert(lua_isfunction(L, -1));
    //push responsibleMutatorIndices argument
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, static_cast<int>(responsibleMutatorIndices.size()), 0);
    //insert each triggerElement into the table for the particular trigger
    int i(0);
    foreach (int mutatorIndex, responsibleMutatorIndices) {
        ++i;
        luaL_checkstack(L, 1, 0);
        lua_pushinteger(L, mutatorIndex);
        lua_rawseti(L, -2, i);
    }
    //push dynamicObject argument
    pushGuy(L, objectToManipulate);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    boost::optional<Guy> retv;
    if (!lua_isnil(L, -1)) {
        assert(lua_istable(L, -1) && "mutated object must be a table");
        retv = to<Guy>(L);
    }
    //pop return value
    lua_pop(L, 1);
    return retv;
}
boost::optional<Box> DirectLuaTriggerFrameState::mutateObject(
    mt::std::vector<int>::type const& responsibleMutatorIndices,
    Box const& objectToManipulate)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "mutateObject");
    luaassert(lua_isfunction(L, -1));
    //push responsibleMutatorIndices argument
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, static_cast<int>(responsibleMutatorIndices.size()), 0);
    //insert each triggerElement into the table for the particular trigger
    int i(0);
    foreach (int mutatorIndex, responsibleMutatorIndices) {
        ++i;
        luaL_checkstack(L, 1, 0);
        lua_pushinteger(L, mutatorIndex);
        lua_rawseti(L, -2, i);
    }
    //push dynamicObject argument
    pushBox(L, objectToManipulate);
    luaL_checkstack(L, 1, 0);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 2, 1);
    //read return value
    boost::optional<Box> retv;
    if (!lua_isnil(L, -1)) {
        assert(lua_istable(L, -1) && "mutated object must be a table");
        retv = toBox(L, static_cast<int>(arrivalLocationsSize_));
    }
    //pop return value
    lua_pop(L, 1);
    return retv;
}

TriggerFrameStateImplementation::DepartureInformation DirectLuaTriggerFrameState::getDepartureInformation(
    mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
    Frame* currentFrame)
{
    lua_State* L(L_.ptr);
    LuaStackManager stack_manager(L);
    //push function to call
    luaL_checkstack(L, 1, 0);
    lua_getglobal(L, "getDepartureInformation");
    luaassert(lua_isfunction(L, -1));
    //push `departures` argument [
    //TODO find out if sparse arrays count as
    //array elements or non-array elements
    luaL_checkstack(L, 1, 0);
    lua_createtable(L, static_cast<int>(departures.size()), 0);
    int i(0);
    foreach (
        ObjectList<Normal> const& departureSection,
        departures | boost::adaptors::map_values)
    {
        ++i;
        luaL_checkstack(L, 1, 0);
        lua_createtable(L, 0, 2);
        {
            luaL_checkstack(L, 1, 0);
            lua_createtable(L, static_cast<int>(departureSection.getList<Guy>().size()), 0);
            int j(0);
            foreach (Guy const& guy, departureSection.getList<Guy>()) {
                ++j;
                pushGuy(L, guy);
                lua_rawseti(L, -2, j);
            }
            lua_setfield(L, -2, "guys");
        }
        {
            luaL_checkstack(L, 1, 0);
            lua_createtable(L, static_cast<int>(departureSection.getList<Box>().size()), 0);
            int j(0);
            foreach (Box const& box, departureSection.getList<Box>()) {
                ++j;
                pushBox(L, box);
                lua_rawseti(L, -2, j);
            }
            lua_setfield(L, -2, "boxes");
        }
        lua_rawseti(L, -2, i);
    }
    //]
    //call function
    lua_call(L, 1, 4);
    
    //read triggers return value
    //Trigger return value looks like:
    /*
    {
        [1] = {13},
        [3] = {100, 1020},
        [4] = {},
        [5] = {2131, 144, 70, 154}
        [trigger index] = {trigger value},
        ...
    }
    */
    mt::std::vector<TriggerData>::type triggers;
    luaassert(lua_istable(L, -4));
    luaL_checkstack(L, 2, 0);
    lua_pushnil(L);
    while (lua_next(L, -5) != 0) {
        luaassert(lua_isnumber(L, -2));
        int index(static_cast<int>(lua_tonumber(L, -2)) - 1);
        mt::std::vector<int>::type value;
        assert(lua_istable(L, -1) && "trigger value must be a table");
        for (std::size_t k(1), end(lua_rawlen(L, -1)); k <= end; ++k) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, k);
            lua_gettable(L, -2);
            assert(lua_isnumber(L, -1));
            value.push_back(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
        triggers.push_back(TriggerData(index, value));
        lua_pop(L, 1);
    }
    //read glitz return value
    //Glitz  return value looks like this:
    /*
    {
        {
            x = <number>,
            y = <number>,
            width = <positive number or 0>,
            height = <positive number or 0>,
            xspeed = <number>,
            yspeed = <number>,
            --these colour numbers are in the range [0, 255]
            --TODO:
            --seriously consider making them be in the range [0, 1]!!
            --This would also warrant a change to the rest of the
            --glitz system throughout the engine.
            forwardsColour = {r = <number>, g = <number>, b = <number>},
            reverseColour = {r = <number>, g = <number>, b = <number>},
            timeDirection = <'forwards' or 'reverse'>
        },
        {
            <as above>
        },
        ...
    }*/

    // get background glitz departure
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type backgroundGlitz;
    if (!lua_isnil(L, -3)) {
        luaassert(lua_istable(L, -3) && "background glitz list must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -3)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -4);
            backgroundGlitz.push_back(toGlitz(L));
            lua_pop(L, 1);
        }
    }
    luaassert(backgroundGlitz.size());
    
    // get forground glitz departure
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type foregroundGlitz;
    if (!lua_isnil(L, -2)) {
        luaassert(lua_istable(L, -2) && "foreground glitz list must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -2)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -3);
            foregroundGlitz.push_back(toGlitz(L));
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
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type newBox;
    if (!lua_isnil(L, -1)) {
        luaassert(lua_istable(L, -1) && "extra boxes list must be a table");
        for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
            luaL_checkstack(L, 1, 0);
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            newBox.push_back(toObjectAndTimeBox(L, currentFrame, arrivalLocationsSize_));
            lua_pop(L, 1);
        }
    }
    
    //pop return values
    lua_pop(L, 4);
    return DepartureInformation(
        calculateActualTriggerDepartures(triggers, triggerOffsetsAndDefaults_, currentFrame),
        backgroundGlitz,
        foregroundGlitz,
        newBox);
}

DirectLuaTriggerFrameState::~DirectLuaTriggerFrameState()
{
}

namespace {
std::vector<char> compileLuaChunk(std::vector<char> const& sourceChunk) {
    std::pair<char const*, char const*> source_iterators;
    if (!sourceChunk.empty()) {
        source_iterators.first = &sourceChunk.front();
        source_iterators.second = &sourceChunk.front() + sourceChunk.size();
    }
    std::vector<char> compiledChunk;
    LuaState L((LuaState::new_state_t()));
    if (lua_load(L.ptr, lua_VectorReader, &source_iterators, "source chunk", 0)) {
        std::cerr << lua_tostring(L.ptr, -1) << std::endl;
    	luaassert(false);
    }
    if(lua_dump(L.ptr, lua_VectorWriter, &compiledChunk)) {
        luaassert(false);
    }
    return compiledChunk;
}
}
DirectLuaTriggerSystem::DirectLuaTriggerSystem(
    std::vector<char> const& triggerSystemLuaChunk,
    std::vector<
            std::pair<
                int,
                std::vector<int>
            >
    > const& triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize) :
        compiledLuaChunk_(compileLuaChunk(triggerSystemLuaChunk)),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize)
{
}
}

