#include "DirectLuaTriggerSystem.h"
#include <cassert>
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "CommonTriggerCode.h"
namespace hg {
namespace {
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

LuaState loadLuaStateFromVector(std::vector<char> const& luaData)
{
    std::pair<char const*, char const*> source_iterators;
    if (!luaData.empty()) {
        source_iterators.first = &luaData.front();
        source_iterators.second = &luaData.front() + luaData.size();
    }
    LuaState L((LuaState::new_state_t()));
    if (lua_load(L.ptr, lua_VectorReader, &source_iterators, "Trigger System")) {
        assert(false);
    }
    luaL_openlibs(L.ptr);
    return L;
}
}

TriggerFrameState DirectLuaTriggerSystem::getFrameState() const
{
    SingleAssignmentPtr<LuaState>& sharedStatePtr(luaStates_->get());
    if (!sharedStatePtr.get()) {
        void* p(multi_thread_operator_new(sizeof(LuaState)));
        try {
            sharedStatePtr = new (p) LuaState();
        }
        catch (...) {
            multi_thread_operator_delete(p);
            throw;
        }
        *sharedStatePtr = loadLuaStateFromVector(compiledLuaChunk_);
    }
    //Unfortunately (due to the lack of perfect forwarding in C++03)
    //this cannot be made into a function.
    //This should be the equivalent to:
    //  return new DirectLuaTriggerFrameState(
    //      luaStates_.get(), triggerOffsetsAndDefaults_, arrivalLocationsSize_);
    //except using a custom allocation function.
    void* p(multi_thread_operator_new(sizeof(DirectLuaTriggerFrameState)));
    try {
        return TriggerFrameState(
            new (p) DirectLuaTriggerFrameState(
                *sharedStatePtr,
                triggerOffsetsAndDefaults_,
                arrivalLocationsSize_));
    }
    catch (...) {
        multi_thread_operator_delete(p);
        throw;
    }
}

DirectLuaTriggerFrameState::DirectLuaTriggerFrameState(
    LuaState& sharedState,
    std::vector<
        std::pair<
            int,
            std::vector<int>
        >
    > const& triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize) :
        L_(sharedState),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize)
{
    lua_pushvalue(L_.ptr, -1);
    lua_call(L_.ptr, 0, 1);
}
namespace {
//Reads the field with name "fieldName" from table at the top of the stack of L
//This field must hold a number representable as an int.
//(behaviour if it isn't to come later)
int readIntField(lua_State* L, char const* fieldName)
{
    lua_getfield(L, -1, fieldName);
    assert(lua_isnumber(L, -1));
    //TODO: better rounding!
    int retv(static_cast<int>(lua_tointeger(L, -1)));
    lua_pop(L, 1);
    return retv;
}
TimeDirection readTimeDirectionField(lua_State* L, char const* fieldName)
{
    lua_getfield(L, -1, fieldName);
    assert(lua_isstring(L, -1));
    char const* timeDirectionString(lua_tostring(L, -1));
    TimeDirection retv;
    if (strcmp(timeDirectionString, "forwards") == 0) {
        retv = FORWARDS;
    }
    else if (strcmp(timeDirectionString, "reverse") == 0) {
        retv = REVERSE;
    }
    else {
        assert(false && "invalid string given as timeDirection");
    }
    lua_pop(L, 1);
    return retv;
}
PickupType readPickupTypeField(lua_State* L, char const* fieldName)
{
    lua_getfield(L, -1, fieldName);
    assert(lua_isstring(L, -1));
    char const* pickupTypeString(lua_tostring(L, -1));
    PickupType retv;
    if (strcmp(pickupTypeString, "timeJump") == 0) {
        retv = timeJump;
    }
    else if (strcmp(pickupTypeString, "reverseTime") == 0) {
        retv = reverseTime;
    }
    else {
        assert(false && "invalid string given as a pickup type");
    }
    lua_pop(L, 1);
    return retv;
}
bool readBooleanField(lua_State* L, char const* fieldName)
{
    lua_getfield(L, -1, fieldName);
    //TODO: better error checking
    assert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    lua_pop(L, 1);
    return retv;
}
//Gives the value of the element at the top of the stack of L,
//interpreted as a Box.
Box toBox(lua_State* L, int arrivalLocationsSize)
{
    assert(lua_istable(L, -1) && "a box must be a table");
    int x(readIntField(L, "x"));
    int y(readIntField(L, "y"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    int size(readIntField(L, "size"));
    int illegalPortal(-1);
    lua_getfield(L, -1, "illegalPortal");
    if (!lua_isnil(L, -1)) {
        assert(lua_isnumber(L, -1));
        illegalPortal = static_cast<int>(lua_tonumber(L, -1)) - 1;
        assert(0 <= illegalPortal);
    }
    lua_pop(L, 1);
    int arrivalBasis(-1);
    lua_getfield(L, -1, "arrivalBasis");
    if (!lua_isnil(L, -1)) {
        assert(lua_isnumber(L, -1));
        arrivalBasis = static_cast<int>(lua_tonumber(L, -1)) - 1;
        assert(0 <= arrivalBasis && arrivalBasis < arrivalLocationsSize);
    }
    lua_pop(L, 1);
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    
    return Box(x, y, xspeed, yspeed, size, illegalPortal, arrivalBasis, timeDirection);
}

Collision toCollision(lua_State* L)
{
    assert(lua_istable(L, -1) && "a collision must be a table");

    int x(readIntField(L, "x"));            
    int y(readIntField(L, "y"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    int width(readIntField(L, "width"));
    int height(readIntField(L, "height"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    
    return Collision(x, y, xspeed, yspeed, width, height, timeDirection);
}
PortalArea toPortal(lua_State* L, std::size_t arrivalLocationsSize)
{
    assert(lua_istable(L, -1) && "a portal must be a table");
    
    int index(readIntField(L, "index") - 1);
    //TODO: better rounding!
    assert(index >= 0);
    
    int x(readIntField(L, "x"));
    int y(readIntField(L, "y"));
    int width(readIntField(L, "width"));
    int height(readIntField(L, "height"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    int destinationIndex(readIntField(L, "destinationIndex") - 1);
    assert(destinationIndex >= 0 && static_cast<std::size_t>(destinationIndex) < arrivalLocationsSize);
    int xDestination(readIntField(L, "xDestination"));
    int yDestination(readIntField(L, "yDestination"));
    bool relativeTime(readBooleanField(L, "relativeTime"));
    int timeDestination(readIntField(L, "timeDestination"));
    
    lua_getfield(L, -1, "illegalDestination");
    int illegalDestination;
    if (lua_isnumber(L, -1)) {
        illegalDestination = static_cast<int>(lua_tointeger(L, -1)) - 1;
        assert(illegalDestination >= 0);
    }
    else {
        assert(lua_isnil(L, -1));
        illegalDestination = -1;
    }
    lua_pop(L, 1);
    
    bool fallable(readBooleanField(L, "fallable"));
    bool winner(readBooleanField(L, "winner"));
    
    return
        PortalArea(
            index,
            x, y,
            width, height,
            xspeed, yspeed,
            timeDirection, destinationIndex,
            xDestination, yDestination,
            relativeTime,
            timeDestination,
            illegalDestination,
            fallable,
            winner);
}
PickupArea toPickup(lua_State* L)
{
    assert(lua_istable(L, -1) && "a pickup must be a table");

    int x(readIntField(L, "x"));            
    int y(readIntField(L, "y"));
    int width(readIntField(L, "width"));
    int height(readIntField(L, "height"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    PickupType type(readPickupTypeField(L, "type"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    
    return PickupArea(x, y, width, height, xspeed, yspeed, type, timeDirection);
}
KillerArea toKiller(lua_State* L)
{
    assert(lua_istable(L, -1) && "a killer must be a table");

    int x(readIntField(L, "x"));            
    int y(readIntField(L, "y"));
    int width(readIntField(L, "width"));
    int height(readIntField(L, "height"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    
    return KillerArea(x, y, width, height, xspeed, yspeed, timeDirection);
}
ArrivalLocation toArrivalLocation(lua_State* L)
{
    int x(readIntField(L, "x"));
    int y(readIntField(L, "y"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    
    return ArrivalLocation(x, y, xspeed, yspeed, timeDirection);
}

unsigned readColourField(lua_State* L, char const* fieldName)
{
    lua_getfield(L, -1, fieldName);
    unsigned r(readIntField(L, "r"));
    unsigned g(readIntField(L, "g"));
    unsigned b(readIntField(L, "b"));
    lua_pop(L, 1);
    return r << 24 | g << 16 | b << 8;
}

RectangleGlitz toGlitz(lua_State* L)
{
    int x(readIntField(L, "x"));
    int y(readIntField(L, "y"));
    int width(readIntField(L, "width"));
    int height(readIntField(L, "height"));
    int xspeed(readIntField(L, "xspeed"));
    int yspeed(readIntField(L, "yspeed"));
    unsigned forwardsColour(readColourField(L, "forwardsColour"));
    unsigned reverseColour(readColourField(L, "reverseColour"));
    TimeDirection timeDirection(readTimeDirectionField(L, "timeDirection"));
    return RectangleGlitz(x, y, width, height, xspeed, yspeed, forwardsColour, reverseColour, timeDirection);
}
}

PhysicsAffectingStuff
    DirectLuaTriggerFrameState::calculatePhysicsAffectingStuff(
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals)
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
            arrivalBasis = <nul or number in the range [1, arrivalLocationsSize]>
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
            timeDirection = <'forwards' or 'backwards'>,
            destinationIndex = <number between 1 and arrivalLocationsSize inclusive>,
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
    //An array-table called "pickups", containing 
    //tables with the following format:
    /*
        {
            x = <number>,
            y = <number>,
            width = <number>,
            height = <number>,
            xspeed = <number>,
            yspeed = <number>,
            type = <'timeJump' or 'reverseTime'>,
            timeDirection = <'forwards' or 'reverse'>
        }
    */
    //An array-table called "killers", containing
    //tables with the following format:
    /*
        {
            x = <number>
            y = <number>
            width = <number>
            height = <number>
            xspeed = <number>
            yspeed = <number>
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
            timeDirection = <'forwards' or 'backwards'>
        }
    */
    //IMPORTANT! - The "arrivalLocations" array-table is compulsary,
    //and must always contain the same number of tables.
    //Furthermore, the "destinationIndex" field of those tables
    //that need it must contain an integer in the range [1, n]
    //where n is that number of tables in arrivalLocations.
    
    mt::std::vector<mt::std::vector<int>::type>::type
        apparentTriggers(calculateApparentTriggers(triggerOffsetsAndDefaults_, triggerArrivals));
        
    PhysicsAffectingStuff retv;

    //***** THIS REGION MUST BE AND REMAIN EXCEPTION FREE!!! ******
    //(that is unless someone figures out how to apply RAII to
    // the lua api)
    //Note that it is currently totally buggy in the face of errors.
    //TODO fix this!
    
    //push function to call
    lua_getfield(L_.ptr, -1, "calculatePhysicsAffectingStuff");
    //push `self` argument
    lua_pushvalue(L_.ptr, -2);
    //push `triggerArrivals` argument [
    lua_createtable(L_.ptr, static_cast<int>(boost::distance(triggerArrivals)), 0);
    //create index and table for each trigger
    int i(0);
    foreach (mt::std::vector<int>::type const& apparentTrigger, apparentTriggers) {
        ++i;
        lua_createtable(L_.ptr, static_cast<int>(apparentTrigger.size()), 0);
        //insert each triggerElement into the table for the particular trigger
        int j(0);
        foreach (int triggerElement, apparentTrigger) {
            ++j;
            lua_pushinteger(L_.ptr, triggerElement);
            lua_rawseti(L_.ptr, -2, j);
        }
        lua_rawseti(L_.ptr, -2, i);
    }
    //]
    //call function

    lua_call(L_.ptr, 2, 1);

    //read return value
    //TODO: better handling of sparsely populated tables.
    
    //read 'additionalBoxes' table:
    lua_getfield(L_.ptr, -1, "additionalBoxes");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(false && "additionalBoxes not yet implemented!");
        assert(lua_istable(L_.ptr, -1) && "additionalBoxes must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            retv.additionalBoxes.push_back(toBox(L_.ptr, static_cast<int>(arrivalLocationsSize_)));
            lua_pop(L_.ptr, 1);
        }
    }
    lua_pop(L_.ptr, 1);
    
    //read 'collisions' table:
    lua_getfield(L_.ptr, -1, "collisions");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(lua_istable(L_.ptr, -1) && "collisions must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            retv.collisions.push_back(toCollision(L_.ptr));
            lua_pop(L_.ptr, 1);
        }
    }
    lua_pop(L_.ptr, 1);
    
    //read 'portals' table
    lua_getfield(L_.ptr, -1, "portals");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(lua_istable(L_.ptr, -1) && "portals must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            retv.portals.push_back(toPortal(L_.ptr, arrivalLocationsSize_));
            lua_pop(L_.ptr, 1);
        }
    }
    lua_pop(L_.ptr, 1);
    
    //read 'pickups' table
    lua_getfield(L_.ptr, -1, "pickups");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(lua_istable(L_.ptr, -1) && "pickups must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            retv.pickups.push_back(toPickup(L_.ptr));
            lua_pop(L_.ptr, 1);
        }
    }
    lua_pop(L_.ptr, 1);
    
    //read 'killers' table
    lua_getfield(L_.ptr, -1, "killers");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(lua_istable(L_.ptr, -1) && "killers must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            retv.killers.push_back(toKiller(L_.ptr));
            lua_pop(L_.ptr, 1);
        }
    }
    lua_pop(L_.ptr, 1);
    //read 'arrivalLocations' table
    lua_getfield(L_.ptr, -1, "arrivalLocations");
    assert(lua_istable(L_.ptr, -1) && "arrivalLocations must be a table");
    for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
        assert(end == arrivalLocationsSize_);
        lua_pushinteger(L_.ptr, i);
        lua_gettable(L_.ptr, -2);
        retv.arrivalLocations.push_back(toArrivalLocation(L_.ptr));
        lua_pop(L_.ptr, 1);
    }
    lua_pop(L_.ptr, 1);
    
    //pop return value
    lua_pop(L_.ptr, 1);
    
    lua_getfield(L_.ptr, -1, "getTriggerDeparturesAndGlitz");
    //assert(lua_isfunction(L_.ptr, -1));
    lua_pop(L_.ptr, 1);
    //***** END EXCEPTION FREE REGION *****
    return retv;
}


namespace {
void pushGuy(lua_State* L, Guy const& guy)
{
    lua_checkstack(L, 1);
    lua_createtable(L, 0, 17);

    lua_checkstack(L, 1);
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
    
    lua_pushboolean(L, guy.getFacing());
    lua_setfield(L, -2, "facing");
    
    lua_createtable(L, static_cast<int>(guy.getPickups().size()), 0);
    typedef std::pair<int const, int> PickupPair;
    foreach (PickupPair const& pickup, guy.getPickups()) {
        lua_checkstack(L, 1);
        lua_pushinteger(L, pickup.second);
        lua_rawseti(L, -2, pickup.first);
    }
    lua_setfield(L, -2, "pickups");
    
    lua_pushboolean(L, guy.getBoxCarrying());
    lua_setfield(L, -2, "boxCarrying");
    if (guy.getBoxCarrying()) {
        lua_checkstack(L, 1);
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
    lua_createtable(L, 0, 10);
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

//TODO: fix code duplication with box version
bool doShouldXFunction(lua_State* L, char const* functionName, int responsibleXIndex, Guy const& potentialXer)
{
    //push function to call
    lua_checkstack(L, 1);
    lua_getfield(L, -1, functionName);
    assert(lua_isfunction(L, -1));
    //push `self` argument
    lua_checkstack(L, 1);
    lua_pushvalue(L, -2);
    //push `responsiblePortalIndex` argument
    lua_checkstack(L, 1);
    lua_pushinteger(L, responsibleXIndex + 1);
    //push `potentialPorter` argument
    lua_checkstack(L, 1);
    pushGuy(L, potentialXer);
    lua_checkstack(L, 1);
    lua_pushstring(L, "guy");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 3, 1);
    //read return value
    assert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
bool doShouldXFunction(lua_State* L, char const* functionName, int responsibleXIndex, Box const& potentialXer)
{
    //push function to call
    lua_checkstack(L, 1);
    lua_getfield(L, -1, functionName);
    assert(lua_isfunction(L, -1));
    //push `self` argument
    lua_checkstack(L, 1);
    lua_pushvalue(L, -2);
    //push `responsiblePortalIndex` argument
    lua_checkstack(L, 1);
    lua_pushinteger(L, responsibleXIndex + 1);
    //push `potentialPorter` argument
    lua_checkstack(L, 1);
    pushBox(L, potentialXer);
    lua_checkstack(L, 1);
    lua_pushstring(L, "box");
    lua_setfield(L, -2, "type");
    //call function
    lua_call(L, 3, 1);
    //read return value
    assert(lua_isboolean(L, -1));
    bool retv(lua_toboolean(L, -1));
    //pop return value
    lua_pop(L, 1);
    return retv;
}
}

//JUST TO BE PERFECTLY CLEAR:
//`responsiblePortalIndex` does not refer to 
//`portal.getIndex()` (which is just for illegal portals)
//it refers to the `i` in `nextPortal[i]`.
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Guy const& potentialPorter)
{
    return doShouldXFunction(L_.ptr, "shouldPort", responsiblePortalIndex, potentialPorter);
}
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Box const& potentialPorter)
{
    return doShouldXFunction(L_.ptr, "shouldPort", responsiblePortalIndex, potentialPorter);
}
    
bool DirectLuaTriggerFrameState::shouldPickup(
    int responsiblePickupIndex,
    Guy const& potentialPickuper)
{
    return doShouldXFunction(L_.ptr, "shouldPickup", responsiblePickupIndex, potentialPickuper);
}
bool DirectLuaTriggerFrameState::shouldPickup(
    int responsiblePickupIndex,
    Box const& potentialPickuper)
{
    return doShouldXFunction(L_.ptr, "shouldPickup", responsiblePickupIndex, potentialPickuper);
}
    
bool DirectLuaTriggerFrameState::shouldDie(
    int responsibleKillerIndex,
    Guy const& potentialDier)
{
    return doShouldXFunction(L_.ptr, "shouldDie", responsibleKillerIndex, potentialDier);
}
bool DirectLuaTriggerFrameState::shouldDie(
    int responsibleKillerIndex,
    Box const& potentialDier)
{
    return doShouldXFunction(L_.ptr, "shouldDie", responsibleKillerIndex, potentialDier);
}

std::pair<
    mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
    mt::std::vector<RectangleGlitz>::type
> 
DirectLuaTriggerFrameState::getTriggerDeparturesAndGlitz(
    mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
    Frame* currentFrame)
{
    //push function to call
    lua_checkstack(L_.ptr, 1);
    lua_getfield(L_.ptr, -1, "getTriggerDeparturesAndGlitz");
    assert(lua_isfunction(L_.ptr, -1));
    //push `self` argument
    lua_checkstack(L_.ptr, 1);
    lua_pushvalue(L_.ptr, -2);
    //push `departures` argument [
    //TODO find out if sparse arrays count as
    //array elements or non-array elements
    lua_checkstack(L_.ptr, 1);
    lua_createtable(L_.ptr, static_cast<int>(departures.size()), 0);
    int i(0);
    typedef std::pair<Frame* const, ObjectList<Normal> > departure_t;
    foreach (departure_t const& departureSection, departures)
    {
        ++i;
        lua_checkstack(L_.ptr, 1);
        lua_createtable(L_.ptr, 0, 2);
        {
            lua_checkstack(L_.ptr, 1);
            lua_createtable(L_.ptr, static_cast<int>(departureSection.second.getList<Guy>().size()), 0);
            int j(0);
            foreach (Guy const& guy, departureSection.second.getList<Guy>()) {
                ++j;
                pushGuy(L_.ptr, guy);
                lua_rawseti(L_.ptr, -2, j);
            }
            lua_setfield(L_.ptr, -2, "guys");
        }
        {
            lua_createtable(L_.ptr, static_cast<int>(departureSection.second.getList<Box>().size()), 0);
            int j(0);
            foreach (Box const& box, departureSection.second.getList<Box>()) {
                ++j;
                pushBox(L_.ptr, box);
                lua_rawseti(L_.ptr, -2, j);
            }
            lua_setfield(L_.ptr, -2, "boxes");
        }
        lua_rawseti(L_.ptr, -2, i);
    }
    //]
    //call function
    lua_call(L_.ptr, 2, 2);
    
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
    assert(lua_istable(L_.ptr, -2));
    lua_pushnil(L_.ptr);
    while (lua_next(L_.ptr, -3) != 0) {
        assert(lua_isnumber(L_.ptr, -2));
        int index(static_cast<int>(lua_tonumber(L_.ptr, -2)) - 1);
        mt::std::vector<int>::type value;
        assert(lua_istable(L_.ptr, -1) && "trigger value must be a table");
        for(std::size_t k(1), end(lua_objlen(L_.ptr, -1)); k <= end; ++k) {
            lua_pushinteger(L_.ptr, k);
            lua_gettable(L_.ptr, -2);
            assert(lua_isnumber(L_.ptr, -1));
            value.push_back(lua_tointeger(L_.ptr, -1));
            lua_pop(L_.ptr, 1);
        }
        triggers.push_back(TriggerData(index, value));
        lua_pop(L_.ptr, 1);
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
    }
    */
    mt::std::vector<RectangleGlitz>::type glitz;
    if (!lua_isnil(L_.ptr, -1)) {
        assert(lua_istable(L_.ptr, -1) && "glitz list must be a table");
        for(std::size_t i(1), end(lua_objlen(L_.ptr, -1)); i <= end; ++i) {
            lua_pushinteger(L_.ptr, i);
            lua_gettable(L_.ptr, -2);
            glitz.push_back(toGlitz(L_.ptr));
            lua_pop(L_.ptr, 1);
        }
    }
    
    //pop return values
    lua_pop(L_.ptr, 2);
    return std::make_pair(calculateActualTriggerDepartures(triggers, triggerOffsetsAndDefaults_, currentFrame), glitz);
}

DirectLuaTriggerFrameState::~DirectLuaTriggerFrameState()
{
    //Pop the table that was returned in the constructor
    lua_pop(L_.ptr, 1);
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
    //TODO - fix security hole here!
    luaL_openlibs(L.ptr);
    if (lua_load(L.ptr, lua_VectorReader, &source_iterators, "source chunk")) {
        assert(false);
    }
    if(lua_dump(L.ptr, lua_VectorWriter, &compiledChunk)) {
        assert(false);
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

