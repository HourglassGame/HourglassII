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


DirectLuaTriggerFrameState::DirectLuaTriggerFrameState(
    std::vector<char> const& compiledLuaChunk,
    std::vector<
        std::pair<
            int,
            std::vector<int>
        >
    > const& triggerOffsetsAndDefaults,
    std::size_t arrivalLocationsSize) :
        L_(loadLuaStateFromVector(compiledLuaChunk)),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize)
{
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
//interpreted as a Collision.
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
    //  ... not specified yet ...
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
    //  ... not specified yet ...
    //An array-table called "killers", containing
    //tables with the following format:
    //  ... not specified yet
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
        assert(false && "pickups not yet implemented!");
    }
    lua_pop(L_.ptr, 1);
    
    //read 'killers' table
    lua_getfield(L_.ptr, -1, "killers");
    if (!lua_isnil(L_.ptr, -1)) {
        assert(false && "killers not yet implemented!");
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

bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Guy const& potentialPorter)
{
#warning not yet implemented
    return true;
}
bool DirectLuaTriggerFrameState::shouldPort(
    int responsiblePortalIndex,
    Box const& potentialPorter)
{
#warning not yet implemented
    return true;
}
    
bool DirectLuaTriggerFrameState::shouldPickup(
    int responsiblePickupIndex,
    Guy const& potentialPickuper)
{
#warning not yet implemented
    return true;
}
bool DirectLuaTriggerFrameState::shouldPickup(
    int responsiblePickupIndex,
    Box const& potentialPickuper)
{
#warning not yet implemented
    return true;
}
    
bool DirectLuaTriggerFrameState::shouldDie(
    int responsibleKillerIndex,
    Guy const& potentialDier)
{
#warning not yet implemented
    return true;
}
bool DirectLuaTriggerFrameState::shouldDie(
    int responsibleKillerIndex,
    Box const& potentialDier)
{
#warning not yet implemented
    return true;
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
                lua_checkstack(L_.ptr, 1);
                lua_createtable(L_.ptr, 0, 17);

                lua_checkstack(L_.ptr, 1);
                lua_pushinteger(L_.ptr, guy.getIndex());
                lua_setfield(L_.ptr, -2, "index");
                lua_pushinteger(L_.ptr, guy.getX());
                lua_setfield(L_.ptr, -2, "x");
                lua_pushinteger(L_.ptr, guy.getY());
                lua_setfield(L_.ptr, -2, "y");
                lua_pushinteger(L_.ptr, guy.getXspeed());
                lua_setfield(L_.ptr, -2, "xspeed");
                lua_pushinteger(L_.ptr, guy.getYspeed());
                lua_setfield(L_.ptr, -2, "yspeed");
                lua_pushinteger(L_.ptr, guy.getWidth());
                lua_setfield(L_.ptr, -2, "width");
                lua_pushinteger(L_.ptr, guy.getHeight());
                lua_setfield(L_.ptr, -2, "height");
                if (guy.getIllegalPortal() != -1) {
                    lua_pushinteger(L_.ptr, guy.getIllegalPortal() + 1);
                    lua_setfield(L_.ptr, -2, "illegalPortal");
                }
                if (guy.getArrivalBasis() != -1) {
                    lua_pushinteger(L_.ptr, guy.getArrivalBasis() + 1);
                    lua_setfield(L_.ptr, -2, "arrivalBasis");
                }
                lua_pushboolean(L_.ptr, guy.getSupported());
                lua_setfield(L_.ptr, -2, "supported");
                if (guy.getSupported()) {
                    lua_pushinteger(L_.ptr, guy.getSupportedSpeed());
                    lua_setfield(L_.ptr, -2, "supportedSpeed");
                }
                
                lua_pushboolean(L_.ptr, guy.getFacing());
                lua_setfield(L_.ptr, -2, "facing");
                
                lua_createtable(L_.ptr, static_cast<int>(guy.getPickups().size()), 0);
                typedef std::pair<int const, int> PickupPair;
                foreach (PickupPair const& pickup, guy.getPickups()) {
                    lua_checkstack(L_.ptr, 1);
                    lua_pushinteger(L_.ptr, pickup.second);
                    lua_rawseti(L_.ptr, -2, pickup.first);
                }
                lua_setfield(L_.ptr, -2, "pickups");
                
                lua_pushboolean(L_.ptr, guy.getBoxCarrying());
                lua_setfield(L_.ptr, -2, "boxCarrying");
                if (guy.getBoxCarrying()) {
                    lua_checkstack(L_.ptr, 1);
                    lua_pushinteger(L_.ptr, guy.getBoxCarrySize());
                    lua_setfield(L_.ptr, -2, "boxCarrySize");
                    lua_pushstring(L_.ptr, guy.getBoxCarryDirection() == FORWARDS ? "forwards" : "reverse");
                    lua_setfield(L_.ptr, -2, "boxCarryDirection");
                }
                lua_pushstring(L_.ptr, guy.getTimeDirection() == FORWARDS ? "forwards" : "reverse");
                lua_setfield(L_.ptr, -2, "timeDirection");
                
                lua_rawseti(L_.ptr, -2, j);
            }
            lua_setfield(L_.ptr, -2, "guyes");
        }
        {
            lua_createtable(L_.ptr, static_cast<int>(departureSection.second.getList<Box>().size()), 0);
            int j(0);
            foreach (Box const& box, departureSection.second.getList<Box>()) {
                ++j;
                lua_createtable(L_.ptr, 0, 8);

                lua_pushinteger(L_.ptr, box.getX());
                lua_setfield(L_.ptr, -2, "x");
                lua_pushinteger(L_.ptr, box.getY());
                lua_setfield(L_.ptr, -2, "y");
                lua_pushinteger(L_.ptr, box.getXspeed());
                lua_setfield(L_.ptr, -2, "xspeed");
                lua_pushinteger(L_.ptr, box.getYspeed());
                lua_setfield(L_.ptr, -2, "yspeed");
                lua_pushinteger(L_.ptr, box.getSize());
                lua_setfield(L_.ptr, -2, "size");
                if (box.getIllegalPortal() != -1) {
                    lua_pushinteger(L_.ptr, box.getIllegalPortal() + 1);
                    lua_setfield(L_.ptr, -2, "illegalPortal");
                }
                if (box.getArrivalBasis() != -1) {
                    lua_pushinteger(L_.ptr, box.getArrivalBasis() + 1);
                    lua_setfield(L_.ptr, -2, "arrivalBasis");
                }
                lua_pushstring(L_.ptr, box.getTimeDirection() == FORWARDS ? "forwards" : "reverse");
                lua_setfield(L_.ptr, -2, "timeDirection");
                
                lua_rawseti(L_.ptr, -2, j);
            }
            lua_setfield(L_.ptr, -2, "boxes");
        }
        lua_rawseti(L_.ptr, -1, i);
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
        [...] = {...},
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
    std::cout << "triggers.size(): " << triggers.size() << "\n";
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
        }
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
    //std::cout << "glitz.size(): " << glitz.size() << "\n";
    
    //pop return values
    lua_pop(L_.ptr, 2);
    return std::make_pair(calculateActualTriggerDepartures(triggers, triggerOffsetsAndDefaults_, currentFrame), glitz);
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

