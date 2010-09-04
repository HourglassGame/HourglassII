/*
 *  WorldState.cpp
 *  HourglassIIGameSFML_Wave
 *
 *  Created by Evan Wallace on 4/09/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "WorldState.h"
using namespace ::hg;
using namespace ::std;

WorldState::WorldState(const TimelineState& timeline,
                       unsigned int timelineLength,
                       FrameID guyStartTime,
                       PhysicsEngine physics,
                       const ObjectList& initialObjects) :
timeline_(timeline),
nextPlayerFrame_(0),/*zero is filler here, I don't have a reasonable `invalid' number*/
currentPlayerFrame_(0),/*zero is filler here, I don't have a reasonable `invalid' number*/
playerInput_(),
frameUpdateSet_(),
physics_(physics)
{
    // boxes
    for (vector<Box>::const_iterator it(initialObjects.getBoxListRef().begin()),
         end(initialObjects.getBoxListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS)
        {
            timeline_.permanentDepartureObjectList(0).addBox(*it);
        }
        else
        {
            timeline_.permanentDepartureObjectList(timelineLength-1).addBox(*it);
        }
    }
    assert(initialObjects.getGuyListRef().size() == 1
           && "This should throw an exception rather than be an assert, but I can't be bothered right now");
    timeline_.permanentDepartureObjectList(guyStartTime).addGuy(initialObjects.getGuyListRef().at(0));
    
    frameUpdateSet_.addFrame(0);
    frameUpdateSet_.addFrame(timelineLength - 1);
}

TimeObjectListList WorldState::getDeparturesFromFrame(const TimelineState::Frame& frame)
{
    // get departures for the frame, update currentPlayerFrame
    // if appropriate 
    TimeObjectListList departures(physics_.executeFrame(frame.getPrePhysics(),
                                                       frame.getTime(),
                                                       playerInput_,
                                                       currentPlayerFrame_,
                                                       nextPlayerFrame_));
    
    departures.sortObjectLists();
    
    return departures;
}


FrameUpdateSet WorldState::executeWorld()
{
    FrameUpdateSet returnSet(frameUpdateSet_);
    typedef map<FrameID, TimeObjectListList> DepartureMap;
    DepartureMap changedFrames;
    for (FrameUpdateSet::const_iterator it(frameUpdateSet_.begin()),
         end(frameUpdateSet_.end()); it != end; ++it) {
        pair<DepartureMap::iterator,bool> ret = changedFrames.insert 
        (
            DepartureMap::value_type 
            (
                *it,
                getDeparturesFromFrame
                (
                    timeline_.getFrame(*it)
                )
            )
        );
        assert(ret.second && "There shouldn't be any duplicates in the frameUpdateSet");
    }
    frameUpdateSet_ = timeline_.updateWithNewDepartures(changedFrames);
    return returnSet;
}

ObjectList WorldState::getPostPhysics(FrameID whichFrame) const
{
    return timeline_.getPostPhysics(whichFrame);
}

void WorldState::addNewInputData(const InputList& newInputData)
{
    playerInput_.push_back(newInputData);
    frameUpdateSet_.addFrame(nextPlayerFrame_);
}

FrameID WorldState::getCurrentPlayerFrame() const
{
    return currentPlayerFrame_;
}
