#include "WorldState.h"
#include "DepartureMap.h"
using namespace ::hg;
using namespace ::std;

WorldState::WorldState(const TimelineState& timeline,
                       unsigned int timelineLength,
                       NewFrameID guyStartTime,
                       PhysicsEngine physics,
                       const ObjectList& initialObjects) :
timeline_(timeline),
nextPlayerFrame_(guyStartTime),
currentPlayerFrame_(),
currentPlayerDirection_(PAUSE),/*filler*/
playerInput_(),
frameUpdateSet_(),
physics_(physics)
{
    assert(nextPlayerFrame_.isValidFrame());
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
    assert(initialObjects.getGuyListRef().size() == 1
           && "This should throw an exception rather than be an assert, but I can't be bothered right now");
    initialArrivalMap[guyStartTime].addGuy(initialObjects.getGuyListRef().at(0));

    TimeObjectListList initialArrivals;

    for (map<NewFrameID, MutableObjectList>::iterator it(initialArrivalMap.begin()),
                                                    end(initialArrivalMap.end()); 
                                                        it != end; 
                                                        ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        initialArrivals.insertObjectList(it->first, ObjectList(it->second));
    }

    timeline_.setArrivalsFromPermanentDepartureFrame(initialArrivals);

    frameUpdateSet_.addFrame(NewFrameID(0, timelineLength));
    frameUpdateSet_.addFrame(NewFrameID(timelineLength - 1, timelineLength));
}

TimeObjectListList WorldState::getDeparturesFromFrame(const TimelineState::Frame& frame)
{
    return physics_.executeFrame(frame.getPrePhysics(),
                                 frame.getTime(),
                                 playerInput_,
                                 currentPlayerFrame_,
                                 currentPlayerDirection_,
                                 nextPlayerFrame_);
}


FrameUpdateSet WorldState::executeWorld()
{
    FrameUpdateSet returnSet(frameUpdateSet_);
    DepartureMap changedFrames;
    changedFrames.reserve(frameUpdateSet_.size());
    for (FrameUpdateSet::const_iterator it(frameUpdateSet_.begin()),
         end(frameUpdateSet_.end()); it != end; ++it)
    {
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
    assert(nextPlayerFrame_.isValidFrame());
    frameUpdateSet_.addFrame(nextPlayerFrame_);
}

NewFrameID WorldState::getCurrentPlayerFrame() const
{
    return currentPlayerFrame_;
}

TimeDirection WorldState::getCurrentPlayerDirection() const
{
    return currentPlayerDirection_;
}
