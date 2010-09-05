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

TimeEngine::TimeEngine(unsigned int timeLineLength,
                       vector<vector<bool> > wallmap,
                       int newWallSize,
                       int newGravity,
                       const ObjectList& initialObjects,
                       FrameID guyStartTime) :
worldState(TimelineState(timeLineLength),
                timeLineLength,
                guyStartTime,
                PhysicsEngine(timeLineLength, wallmap, newWallSize, newGravity),
                initialObjects)

{
    //** run level for a while
    for (unsigned int i = 0; i < timeLineLength; ++i) {
        worldState.executeWorld();
    }
}

tuple<FrameID, TimeEngine::FrameListList> TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    worldState.addNewInputData(newInputData);

    FrameListList updatedList;
    unsigned const int speedOfTime = 3; // speed at which waves propagate in frames per step.
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(worldState.executeWorld());
    }
    return tuple<FrameID, FrameListList>(worldState.getCurrentPlayerFrame(), updatedList);
}

ObjectList TimeEngine::getPostPhysics(FrameID whichFrame) const
{
    return worldState.getPostPhysics(whichFrame);
}
