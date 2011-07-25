#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

#include <boost/swap.hpp>

namespace hg {
TimeEngine::TimeEngine(Level const& level) :
        speedOfTime_(level.speedOfTime),
        worldState_(level.timeLineLength,
                   level.guyStartTime,
                   PhysicsEngine(level.environment, level.newOldTriggerSystem),
                   level.initialObjects)
{
    
}

void TimeEngine::swap(TimeEngine& other) {
    boost::swap(speedOfTime_, other.speedOfTime_);
    boost::swap(worldState_, other.worldState_);
}

TimeEngine::RunResult
TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState_.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(speedOfTime_);
    for (unsigned int i(0); i < speedOfTime_; ++i) {
        updatedList.push_back(worldState_.executeWorld());
    }
    return RunResult(
        worldState_.getCurrentPlayerFrame(),
        worldState_.getNextPlayerFrame(),
        updatedList);
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
