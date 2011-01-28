#include "TimeEngine.h"

#include "InvalidLevelException.h"
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
worldState(level.timeLineLength,
           level.guyStartTime,
           PhysicsEngine(level.wallmap, level.wallSize, level.gravity, level.attachmentMap, level.triggerSystem),
           level.initialObjects)

{
}

tuple<FrameID,FrameID, TimeEngine::FrameListList, TimeDirection> TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    FrameListList updatedList;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return tuple<FrameID, FrameID, FrameListList, TimeDirection>(worldState.getCurrentPlayerFrame(), worldState.getNextPlayerFrame(), updatedList, worldState.getCurrentPlayerDirection());
}

ObjectList TimeEngine::getPostPhysics(FrameID whichFrame, const PauseInitiatorID& whichPrePause) const
{
    return worldState.getPostPhysics(whichFrame, whichPrePause);
}
    
::std::vector<InputList> TimeEngine::getReplayData() const
{
    return worldState.getReplayData();
}
}//namespace hg
