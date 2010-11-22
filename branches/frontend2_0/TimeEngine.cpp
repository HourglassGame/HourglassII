#include "TimeEngine.h"

#include "InvalidLevelException.h"
#include "ParadoxException.h"
#include "ObjectList.h"
#include "Level.h"

#include <iostream>
#include <cassert>
#include <algorithm>

using namespace ::std;
using namespace ::boost;
namespace hg {
TimeEngine::TimeEngine(const Level& level) :
speedOfTime(level.speedOfTime),
worldState(TimelineState(level.timeLineLength),
                level.timeLineLength,
                level.guyStartTime,
                PhysicsEngine(level.wallmap, level.wallSize, level.gravity, level.attachmentMap, level.triggerSystem),
                level.initialObjects)

{

}

RunResult TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    RunResult::FrameListList updatedList;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return RunResult(worldState.getCurrentPlayerFrame(), worldState.getNextPlayerFrame(), updatedList, worldState.getCurrentPlayerDirection());
}

ObjectList TimeEngine::getPostPhysics(NewFrameID whichFrame, const PauseInitiatorID& whichPrePause) const
{
    return worldState.getPostPhysics(whichFrame, whichPrePause);
}
    
::std::vector<InputList> TimeEngine::getReplayData() const
{
    return worldState.getReplayData();
}
}//namespace hg
