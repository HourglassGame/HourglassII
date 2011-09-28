#include "LevelLoader.h"
#include <fstream>
#include <vector>
#include "SimpleLuaCpp.h"
#include "lua/lualib.h"
#include "LuaUtilities.h"
namespace hg {

Level loadLevelFromFile(std::string const& filename)
{
    std::vector<char> levelChunk(loadFileIntoVector(filename));
    LuaState levelGenerator(loadLuaStateFromVector(levelChunk, "Level"));
    lua_State* L(levelGenerator.ptr);
    //Call function
    lua_call(L, 0, 1);

    //read return [
    //read speedOfTime
    unsigned speedOfTime(readField<int>(L, "speedOfTime"));
    //read timelineLength
    std::size_t timelineLength(readField<int>(L, "timelineLength"));
    //read environment [
    Environment environment(readField<Environment>(L, "environment"));

    //] read environment
    //read initial objects [
    ObjectList<NonGuyDynamic> initialArrivals(readField<InitialObjects>(L, "initialArrivals").list);
    //] read initial objects
    //read initial guy [
    InitialGuyArrival initialGuy(readField<InitialGuyArrival>(L, "initialGuy"));
    Guy const& guyArrival(initialGuy.arrival);
    FrameID guyStartTime(initialGuy.arrivalTime, UniverseID(timelineLength));
    //] read initial guy
    //read trigger system [
    TriggerSystem triggerSystem(readField<TriggerSystem>(L, "triggerSystem"));
    //] read trigger system
    //] read return
    //pop return
    lua_pop(L, 1);
    return Level(speedOfTime, timelineLength, environment, initialArrivals, guyArrival, guyStartTime, triggerSystem);
}
} //namespace hg
