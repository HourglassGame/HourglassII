#include "WorldState.h"
#include "DepartureMap.h"
#include <iostream>

using namespace ::std;
namespace hg {
WorldState::WorldState(const TimelineState& timeline,
                       unsigned int timelineLength,
                       NewFrameID guyStartTime,
                       PhysicsEngine physics,
                       const ObjectList& initialObjects) :
timeline_(timeline),
nextPlayerFrame_(guyStartTime),
currentPlayerFrame_(),
currentPlayerDirection_(INVALID),/*filler*/
playerInput_(),
frameUpdateSet_(),
physics_(physics),
currentWinFrame_()
{
    
    assert(nextPlayerFrame_.isValidFrame());
    map<NewFrameID, MutableObjectList> initialPlatformArrivalMap;

    for (vector<Platform>::const_iterator it(initialObjects.getPlatformListRef().begin()),
         end(initialObjects.getPlatformListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialPlatformArrivalMap[NewFrameID(0, timelineLength)].addPlatform(*it);
        }
        else {
            initialPlatformArrivalMap[NewFrameID(timelineLength-1, timelineLength)].addPlatform(*it);
        }
    }

    TimeObjectListList initialPlatformArrivals;

    for (map<NewFrameID, MutableObjectList>::iterator it(initialPlatformArrivalMap.begin()),
                                                    end(initialPlatformArrivalMap.end());
                                                        it != end;
                                                        ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        initialPlatformArrivals.insertObjectList(it->first, ObjectList(it->second));
    }

    timeline_.addArrivalsFromPermanentDepartureFrame(initialPlatformArrivals);
    frameUpdateSet_.addFrame(NewFrameID(0, timelineLength));
    frameUpdateSet_.addFrame(NewFrameID(timelineLength - 1, timelineLength));

    //** run level for a while
    for (unsigned int i = 0; i < timelineLength; ++i) {
        executeWorld();
    }

    map<NewFrameID, MutableObjectList> initialArrivalMap;

    for (vector<Box>::const_iterator it(initialObjects.getBoxListRef().begin()),
         end(initialObjects.getBoxListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialArrivalMap[NewFrameID(0, timelineLength)].addBox(*it);
        }
        else {
            initialArrivalMap[NewFrameID(timelineLength-1, timelineLength)].addBox(*it);
        }
    }

    for (vector<Button>::const_iterator it(initialObjects.getButtonListRef().begin()),
         end(initialObjects.getButtonListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialArrivalMap[NewFrameID(0, timelineLength)].addButton(*it);
        }
        else {
            initialArrivalMap[NewFrameID(timelineLength-1, timelineLength)].addButton(*it);
        }
    }

    TimeObjectListList initialArrivals;

    assert(initialObjects.getGuyListRef().size() == 1
           && "This should throw an exception rather than be an assert, but I can't be bothered right now");
    initialArrivalMap[guyStartTime].addGuy(initialObjects.getGuyListRef().at(0));

    for (map<NewFrameID, MutableObjectList>::iterator it(initialArrivalMap.begin()),
                                                    end(initialArrivalMap.end());
                                                        it != end;
                                                        ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        initialArrivals.insertObjectList(it->first, ObjectList(it->second));
    }

    timeline_.addArrivalsFromPermanentDepartureFrame(initialArrivals);

    frameUpdateSet_.addFrame(NewFrameID(0, timelineLength));
    frameUpdateSet_.addFrame(NewFrameID(timelineLength - 1, timelineLength));

    //** run level for a while
    for (unsigned int i = 0; i < timelineLength; ++i) {
        executeWorld();
    }
}

TimeObjectListList WorldState::getDeparturesFromFrame(const TimelineState::Frame& frame)
{
    if (frame.getTime() == currentWinFrame_) {
        currentWinFrame_ == NewFrameID();
    }
    return physics_.executeFrame(frame.getPrePhysics(),
                                 frame.getTime(),
                                 playerInput_,
                                 currentPlayerFrame_,
                                 currentPlayerDirection_,
                                 nextPlayerFrame_,
                                 currentWinFrame_);
}


FrameUpdateSet WorldState::executeWorld()
{
    //cout << "executeWorld\n";
    FrameUpdateSet returnSet(frameUpdateSet_);
    DepartureMap changedFrames;
    changedFrames.reserve(frameUpdateSet_.size());
    for (FrameUpdateSet::const_iterator it(frameUpdateSet_.begin()),
         end(frameUpdateSet_.end()); it != end; ++it)
    {
    //    cout << "executing frame: " << it->frame() << "\n";
        changedFrames.addDeparture(*it, getDeparturesFromFrame(timeline_.getFrame(*it)));
    }
    frameUpdateSet_ = timeline_.updateWithNewDepartures(changedFrames);
    return returnSet;
}

ObjectList WorldState::getPostPhysics(NewFrameID whichFrame) const
{
    return timeline_.getPostPhysics(whichFrame);
}

void WorldState::addNewInputData(const InputList& newInputData)
{
    playerInput_.push_back(newInputData);
    if (nextPlayerFrame_.isValidFrame()) {
        frameUpdateSet_.addFrame(nextPlayerFrame_);
        //cout << "adding frame: " << nextPlayerFrame_.frame() << "\n";
    }
    currentPlayerDirection_ = INVALID;
    currentPlayerFrame_ = NewFrameID();
    nextPlayerFrame_ = NewFrameID();
}

NewFrameID WorldState::getCurrentPlayerFrame() const
{
    return currentPlayerFrame_;
}

TimeDirection WorldState::getCurrentPlayerDirection() const
{
    return currentPlayerDirection_;
}
}//namespace hg
