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
    lua_call(L, 0, 0);

    //read the appropriate globals [
    //read speedOfTime
    unsigned speedOfTime(readGlobal<int>(L, "speedOfTime"));
    //read timelineLength
    std::size_t timelineLength(readGlobal<int>(L, "timelineLength"));
    //read environment [
    Environment environment(readGlobal<Environment>(L, "environment"));
    //] read environment
    //read initial objects [
    ObjectList<NonGuyDynamic> initialArrivals(readGlobal<InitialObjects>(L, "initialArrivals").list);
    //] read initial objects
    //read initial guy [
    InitialGuyArrival initialGuy(readGlobal<InitialGuyArrival>(L, "initialGuy"));
    Guy const& guyArrival(initialGuy.arrival);
    FrameID guyStartTime(initialGuy.arrivalTime, UniverseID(timelineLength));
    //] read initial guy
    //read trigger system [
    TriggerSystem triggerSystem(readGlobal<TriggerSystem>(L, "triggerSystem"));
    //] read trigger system
    //] read globals
    return Level(speedOfTime, timelineLength, environment, initialArrivals, guyArrival, guyStartTime, triggerSystem);
}
} //namespace hg
