#include "TimeEngine.h"

#include "InvalidLevelException.h"
#include "ParadoxException.h"
#include "ObjectList.h"

#include <boost/foreach.hpp>

#include <iostream>
#include <cassert>
#include <algorithm>

#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace ::std;
using namespace ::hg;
using namespace ::boost;

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

tuple<NewFrameID, TimeEngine::FrameListList, TimeDirection> TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    FrameListList updatedList;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return tuple<NewFrameID, FrameListList, TimeDirection>(worldState.getCurrentPlayerFrame(), updatedList, worldState.getCurrentPlayerDirection());
}

ObjectList TimeEngine::getPostPhysics(NewFrameID whichFrame) const
{
    return worldState.getPostPhysics(whichFrame);
}
