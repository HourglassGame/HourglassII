#include "TimeEngine.h"

#include "InvalidLevelException.h"
#include "ParadoxException.h"
#include "ObjectList.h"

#include <iostream>
#include <cassert>
#include <algorithm>

using namespace ::std;
using namespace ::boost;
namespace hg {
TimeEngine::TimeEngine(unsigned int nspeedOfTime,
                       unsigned int timeLineLength,
                       vector<vector<bool> > wallmap,
                       int newWallSize,
                       int newGravity,
                       const ObjectList& initialObjects,
                       NewFrameID guyStartTime,
                       AttachmentMap nAttachmentMap,
                       TriggerSystem nTriggerSystem) :
speedOfTime(nspeedOfTime),
worldState(TimelineState(timeLineLength),
                timeLineLength,
                guyStartTime,
                PhysicsEngine(wallmap, newWallSize, newGravity, nAttachmentMap, nTriggerSystem),
                initialObjects)

{

}

tuple<NewFrameID,NewFrameID, TimeEngine::FrameListList, TimeDirection> TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    FrameListList updatedList;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return tuple<NewFrameID, NewFrameID, FrameListList, TimeDirection>(worldState.getCurrentPlayerFrame(), worldState.getNextPlayerFrame(), updatedList, worldState.getCurrentPlayerDirection());
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