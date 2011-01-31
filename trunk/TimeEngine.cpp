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

TimeEngine::RunResult TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    FrameListList updatedList;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    TimeEngine::RunResult retv =  {worldState.getCurrentPlayerFrame(), worldState.getNextPlayerFrame(), updatedList, worldState.getCurrentPlayerDirection()};
    return retv;
}
    
::std::vector<InputList> TimeEngine::getReplayData() const
{
    return worldState.getReplayData();
}

Frame* TimeEngine::getFrame(const FrameID& whichFrame)
{
    return worldState.getFrame(whichFrame);
}
}//namespace hg
