#include <iostream>
#include <cassert>

#include "TimeEngine.h"

#include "InvalidLevelException.h"
#include "ParadoxException.h"
#include "ObjectList.h"

#include <boost/foreach.hpp>
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
endOfFrameState(TimelineState(timeLineLength),timeLineLength,guyStartTime),
playerInput(),
physics(timeLineLength, wallmap, newWallSize, newGravity)
{
    // boxes
    for (vector<Box>::const_iterator it(initialObjects.getBoxListRef().begin()),
         end(initialObjects.getBoxListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS)
        {
            endOfFrameState.timeline.permanentDepartureObjectList(0).addBox(*it);
        }
        else
        {
            endOfFrameState.timeline.permanentDepartureObjectList(timeLineLength-1).addBox(*it);
        }
    }
    assert(initialObjects.getGuyListRef().size() == 1
           && "This should throw an exception rather than be an assert, but I can't be bothered right now");
    endOfFrameState.timeline.permanentDepartureObjectList(guyStartTime).addGuy(initialObjects.getGuyListRef().at(0));

    endOfFrameState.frameUpdateList.push_back(0);
    endOfFrameState.frameUpdateList.push_back(timeLineLength - 1);

    //** run level for a while
    for (unsigned int i = 0; i < timeLineLength; ++i) {
        executeWorld(endOfFrameState);
    }
}

tuple<FrameID, TimeEngine::FrameListList> TimeEngine::runToNextPlayerFrame(const InputList& newInputData)
{
    playerInput.push_back(newInputData);
    endOfFrameState.frameUpdateList.push_back(endOfFrameState.nextPlayerFrame);
    FrameListList updatedList;
    //Leaving out variable speed and frame-specific speed in the interest of getting the initial cut done
    //Adding it may require significant changes ;_;, but anyway...
    unsigned const int speedOfTime = 3;
    for (unsigned int i = 0; i < speedOfTime; ++i) {
        updatedList.push_back(endOfFrameState.frameUpdateList);
        executeWorld(endOfFrameState);
    }
    return tuple<FrameID, FrameListList>(endOfFrameState.currentPlayerFrame, updatedList);
}

void TimeEngine::executeWorld(WorldState& currentState) const
{
    typedef map<FrameID, TimeObjectListList> DepartureMap;
    DepartureMap changedFrames;
    //WorldState newWorldState(currentState);
    foreach (FrameID frame, currentState.frameUpdateList) {
        pair<DepartureMap::iterator,bool> ret = changedFrames.insert (
            DepartureMap::value_type (
                frame,
                getDeparturesFromFrame(currentState.timeline.getFrame(frame), currentState.currentPlayerFrame, currentState.nextPlayerFrame)
            )
        );
        assert(ret.second && "There shouldn't be any duplicates in the frameUpdateList");
    }
    currentState.frameUpdateList = currentState.timeline.updateWithNewDepartures(changedFrames);
}

ObjectList TimeEngine::getPostPhysics(FrameID whichFrame) const
{
    return endOfFrameState.timeline.getPostPhysics(whichFrame);
}

TimeObjectListList TimeEngine::getDeparturesFromFrame(const TimelineState::Frame& frame, FrameID& currentPlayerFrame, FrameID& nextPlayerFrame) const
{
    // get departures for the frame, update currentPlayerFrame
    // if appropriate
    TimeObjectListList departures(physics.executeFrame(frame.getPrePhysics(),
                                                       frame.getTime(),
                                                       playerInput,
                                                       currentPlayerFrame,
                                                       nextPlayerFrame));

    // update departures from this frame
    departures.sortObjectLists();
    return departures;
}
