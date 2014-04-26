#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

#include <boost/swap.hpp>

namespace hg {
TimeEngine::TimeEngine(Level &&level, OperationInterrupter &interrupter) :
        speedOfTime(level.speedOfTime),
        worldState(
            level.timelineLength,
            std::move(level.initialGuy),
            std::move(level.guyStartTime),
            PhysicsEngine(Environment(level.environment), std::move(level.triggerSystem)),
            std::move(level.initialObjects),
            interrupter),
        wall(level.environment.wall)
{
}

void TimeEngine::swap(TimeEngine &o) noexcept {
    boost::swap(speedOfTime, o.speedOfTime);
    boost::swap(worldState, o.worldState);
    boost::swap(wall, o.wall);
}

TimeEngine::RunResult
TimeEngine::runToNextPlayerFrame(InputList const &newInputData, OperationInterrupter &interrupter)
{
    worldState.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(speedOfTime);
    for (unsigned int i(0); i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld(interrupter));
    }
    return RunResult{
        worldState.getCurrentPlayerFrame(),
        worldState.getNextPlayerFrame(),
        std::move(updatedList)};
}

}//namespace hg
