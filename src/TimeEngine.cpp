#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

#include <boost/swap.hpp>

namespace hg {
TimeEngine::TimeEngine(BOOST_RV_REF(Level) level/*, OperationInterrupter& interrupter*/) :
        speedOfTime_(level.speedOfTime),
        worldState_(
            level.timelineLength,
            boost::move(level.initialGuy),
            boost::move(level.guyStartTime),
            PhysicsEngine(Environment(level.environment), boost::move(level.triggerSystem)),
            boost::move(level.initialObjects)/*,
            interrupter*/),
        wall_(level.environment.wall)
{
}

void TimeEngine::swap(TimeEngine& other) {
    boost::swap(speedOfTime_, other.speedOfTime_);
    boost::swap(worldState_, other.worldState_);
    boost::swap(wall_, other.wall_);
}

TimeEngine::RunResult
TimeEngine::runToNextPlayerFrame(const InputList& newInputData/*, OperationInterrupter& interrupter*/)
{
    worldState_.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(speedOfTime_);
    for (unsigned int i(0); i < speedOfTime_; ++i) {
        updatedList.push_back(worldState_.executeWorld(/*interrupter*/));
    }
    return RunResult(
        worldState_.getCurrentPlayerFrame(),
        worldState_.getNextPlayerFrame(),
        boost::move(updatedList));
}

std::vector<InputList> const& TimeEngine::getReplayData() const
{
    return worldState_.getReplayData();
}

Frame* TimeEngine::getFrame(const FrameID& whichFrame)
{
    return worldState_.getFrame(whichFrame);
}
}//namespace hg
