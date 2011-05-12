#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

namespace hg {
TimeEngine::TimeEngine(const Level& level) :
        speedOfTime(level.speedOfTime),
        worldState(level.timeLineLength,
                   level.guyStartTime,
                   PhysicsEngine(level.environment, level.triggerSystem),
                   level.initialObjects)

{
}

TimeEngine::RunResult TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);
    updatedList.clear();
    updatedList.resize(speedOfTime);
    for (unsigned int i(0); i < speedOfTime; ++i) {
        worldState.executeWorld().swap(updatedList[i]);
    }
    TimeEngine::RunResult retv;
    retv.currentPlayerFrame_ = worldState.getCurrentPlayerFrame();
    retv.nextPlayerFrame_ = worldState.getNextPlayerFrame();
    retv.updatedFrames_ = &updatedList;
    return retv;
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