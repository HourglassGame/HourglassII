#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

namespace hg {
TimeEngine::TimeEngine(BOOST_RV_REF(Level) level) :
        speedOfTime(level.speedOfTime),
        worldState(level.timeLineLength,
                   level.guyStartTime,
                   PhysicsEngine(boost::move(level.environment), level.newOldTriggerSystem),
                   level.initialObjects)

{
}
TimeEngine::TimeEngine(BOOST_RV_REF(TimeEngine) o) :
    speedOfTime(o.speedOfTime),
    worldState(boost::move(o.worldState))
{
}
TimeEngine& TimeEngine::operator=(BOOST_RV_REF(TimeEngine) o)
{
    speedOfTime = o.speedOfTime;
    worldState = boost::move(o.worldState);
    return *this;
}

TimeEngine::RunResult
TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(speedOfTime);
    for (unsigned int i(0); i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return RunResult(
        worldState.getCurrentPlayerFrame(),
        worldState.getNextPlayerFrame(),
        boost::move(updatedList));
}

std::vector<InputList> TimeEngine::getReplayData() const
{
    return worldState.getReplayData();
}

Frame* TimeEngine::getFrame(const FrameID& whichFrame)
{
    return worldState.getFrame(whichFrame);
}
}//namespace hg
