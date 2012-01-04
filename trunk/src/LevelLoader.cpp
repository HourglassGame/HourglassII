#include "LevelLoader.h"
#include <fstream>
#include <vector>
#include "SimpleLuaCpp.h"
#include "lua/lualib.h"
#include "LuaUtilities.h"
namespace hg {

Level loadLevelFromFile(
    std::string const& filename,
    OperationInterruptor const& /*interruptor*/)
{
    std::vector<char> levelChunk(loadFileIntoVector(filename));
    LuaState levelGenerator(loadLuaStateFromVector(levelChunk, "Level"));
    lua_State* L(levelGenerator.ptr);

    lua_call(L, 0, 0);

    unsigned speedOfTime(readGlobal<int>(L, "speedOfTime"));
    std::size_t timelineLength(readGlobal<int>(L, "timelineLength"));
    Environment environment(readGlobal<Environment>(L, "environment"));
    ObjectList<NonGuyDynamic> initialArrivals(readGlobal<InitialObjects>(L, "initialArrivals").list);
    InitialGuyArrival initialGuy(readGlobal<InitialGuyArrival>(L, "initialGuy"));
    Guy& guyArrival(initialGuy.arrival);
    FrameID guyStartTime(initialGuy.arrivalTime, UniverseID(timelineLength));
    TriggerSystem triggerSystem(readGlobal<TriggerSystem>(L, "triggerSystem"));

    return Level(
    	boost::move(speedOfTime),
    	boost::move(timelineLength),
    	boost::move(environment),
    	boost::move(initialArrivals),
    	boost::move(guyArrival),
    	boost::move(guyStartTime),
    	boost::move(triggerSystem));
}
} //namespace hg
