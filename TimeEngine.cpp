#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

#include <boost/swap.hpp>
#include "move.h"

namespace hg {
TimeEngine::TimeEngine(Level&& level/*, ProgressMonitor& monitor*/) :
        speedOfTime_(level.speedOfTime),
        worldState_(
            level.timelineLength,
            hg::move(level.initialGuy),
            hg::move(level.guyStartTime),
            PhysicsEngine(level.environment, hg::move(level.triggerSystem)),
            hg::move(level.initialObjects)/*,
            monitor*/),
        wall_(level.environment.wall)
{
    
}

void TimeEngine::swap(TimeEngine& other) {
    boost::swap(speedOfTime_, other.speedOfTime_);
    boost::swap(worldState_, other.worldState_);
    boost::swap(wall_, other.wall_);
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
        hg::move(updatedList));
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
