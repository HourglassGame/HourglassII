#include "WorldState.h"
#include "DepartureMap.h"
#include "PlayerVictoryException.h"
#include "ParallelForEach.h"
//#include <boost/range/algorithm/for_each.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#define foreach BOOST_FOREACH
using namespace ::std;
namespace hg {

struct ExecuteFrame
{
    ExecuteFrame(WorldState& thisptr, DepartureMap& departuremap):
    thisptr_(thisptr),
    departuremap_(departuremap)
    {
    }
    void operator()(const FrameID& time) const
    {
        departuremap_.addDeparture(time, thisptr_.getDeparturesFromFrame(thisptr_.timeline_.getFrame(time)));
    }
    private:
    WorldState& thisptr_;
    DepartureMap& departuremap_;
};



WorldState::WorldState(unsigned int timelineLength,
                       FrameID guyStartTime,
                       PhysicsEngine physics,
                       const ObjectList& initialObjects) :
timeline_(),
playerInput_(),
frameUpdateSet_(),
physics_(physics),
nextPlayerFrames_(),
currentPlayerFramesAndDirections_(),
currentWinFrames_()
{

    assert(guyStartTime.isValidFrame());
    nextPlayerFrames_.add(guyStartTime);
    //*** Add Platforms to world ***
    map<FrameID, MutableObjectList> initialPlatformArrivalMap;

    for (vector<Platform>::const_iterator it(initialObjects.getPlatformListRef().begin()),
         end(initialObjects.getPlatformListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialPlatformArrivalMap[FrameID(0, timelineLength)].add(*it);
        }
        else {
            initialPlatformArrivalMap[FrameID(timelineLength-1, timelineLength)].add(*it);
        }
    }

    TimeObjectListList initialPlatformArrivals;

    for (map<FrameID, MutableObjectList>::iterator it(initialPlatformArrivalMap.begin()),
                                                    end(initialPlatformArrivalMap.end());
                                                        it != end;
                                                        ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        initialPlatformArrivals.insertObjectList(it->first, ObjectList(it->second));
    }

    timeline_.addArrivalsFromPermanentDepartureFrame(initialPlatformArrivals);

    // run level from both ends, platforms can be the ONLY objects in the world at this point as attachment must always have a target
    frameUpdateSet_.addFrame(FrameID(0, timelineLength));
    frameUpdateSet_.addFrame(FrameID(timelineLength - 1, timelineLength));

    //** run level for a while
    for (unsigned int i = 0; i < timelineLength; ++i) {
        executeWorld();
    }

    //** Add everything else apart from guys **
    map<FrameID, MutableObjectList> initialArrivalMap;

    // boxes
    for (vector<Box>::const_iterator it(initialObjects.getBoxListRef().begin()),
         end(initialObjects.getBoxListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialArrivalMap[FrameID(0, timelineLength)].add(*it);
        }
        else {
            initialArrivalMap[FrameID(timelineLength-1, timelineLength)].add(*it);
        }
    }

    // portals
    for (vector<Portal>::const_iterator it(initialObjects.getPortalListRef().begin()),
         end(initialObjects.getPortalListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialArrivalMap[FrameID(0, timelineLength)].add(*it);
        }
        else {
            initialArrivalMap[FrameID(timelineLength-1, timelineLength)].add(*it);
        }
    }

    // buttons
    for (vector<Button>::const_iterator it(initialObjects.getButtonListRef().begin()),
         end(initialObjects.getButtonListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS) {
            initialArrivalMap[FrameID(0, timelineLength)].add(*it);
        }
        else {
            initialArrivalMap[FrameID(timelineLength-1, timelineLength)].add(*it);
        }
    }

    TimeObjectListList initialArrivals;

    assert(initialObjects.getGuyListRef().size() == 1
           && "This should throw an exception rather than be an assert, but I can't be bothered right now");
    initialArrivalMap[guyStartTime].add(initialObjects.getGuyListRef().at(0));

    for (map<FrameID, MutableObjectList>::iterator it(initialArrivalMap.begin()),
                                                    end(initialArrivalMap.end());
                                                        it != end;
                                                        ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        initialArrivals.insertObjectList(it->first, ObjectList(it->second));
    }

    timeline_.addArrivalsFromPermanentDepartureFrame(initialArrivals);

    frameUpdateSet_.addFrame(FrameID(0, timelineLength));
    frameUpdateSet_.addFrame(FrameID(timelineLength - 1, timelineLength));
    //Guys without input can still affect stuff, and so must be run
    frameUpdateSet_.addFrame(guyStartTime);
    //** run level for a while
    for (unsigned int i = 0; i < timelineLength; ++i) {
        executeWorld();
    }
}

TimeObjectListList WorldState::getDeparturesFromFrame(const TimelineState::Frame& frame)
{
    return physics_.executeFrame(frame.getPrePhysics(),
                                 frame.getTime(),
                                 playerInput_,
                                 currentPlayerFramesAndDirections_,
                                 nextPlayerFrames_,
                                 currentWinFrames_);
}

std::vector<FrameID> WorldState::executeWorld()
{
    //cout << "executeWorld\n";
    //Make parallel_for_each compatible type, also save result for return
    //(I don't really understand the mechanism, but tbb::parallel_for_each doesn't work for FrameUpdatSets, but does for vectors)
    std::vector<FrameID> returnSet(frameUpdateSet_.begin(), frameUpdateSet_.end());
    DepartureMap changedFrames;
    changedFrames.makeSpaceFor(frameUpdateSet_);
    parallel_for_each(returnSet, ExecuteFrame(*this, changedFrames));
    //boost::for_each(frameUpdateSet_, ExecuteFrame(*this, changedFrames));
    frameUpdateSet_ = timeline_.updateWithNewDepartures(changedFrames);
    if (frameUpdateSet_.empty() && currentWinFrames_.size() == 1) {
        throw PlayerVictoryException();
    }
    return returnSet;
}

ObjectList WorldState::getPostPhysics(FrameID whichFrame, const PauseInitiatorID& whichPrePause) const
{
    return timeline_.getPostPhysics(whichFrame, whichPrePause);
}


//The worrying situation is when there are arrivals at a frame which are equal to arrivals which happened earlier,
//except that the effects of the new arrivals are different from the old ones because playerInput_.Count is larger
//than it was before. This situation breaks the wave assumption that departures == apply_physics(arrivals).

//this is ok? because a new arrival with maxGuyIndex >= playerInput_.size() is either:
//			run, in which case nextPlayerFrames_ will be set and the frame will be run again when new input becomes available
//		or
//			not run, in which case it will be still sitting in frameUpdateSet_ and will be run in the next executeWorld
//The second case depends on the new arrival triggering placement in frameUpdateSet_,
// which would not happen if that arrival was replacing an earlier equivalent arrival
//However, in this case it would be ok because the earlier equivalent arrival must already run
// and added itself to nextPlayerFrames_
//The basic assertion in the whole system is maxGuyIndex <= playerInput_.size()
 
//A better? system would remove physics from having to do any work at all, and would have the timeline manage this 
//together with the WorldState by scanning all new arrivals.
/**
* Stores the given input data, allowing the player to exist for another step.
*/
void WorldState::addNewInputData(const InputList& newInputData)
{
    playerInput_.push_back(newInputData);
    foreach(const FrameID& frame, nextPlayerFrames_) {
        frameUpdateSet_.addFrame(frame);
        //cout << "adding frame: " << nextPlayerFrame_.frame() << "\n";
    }
    //All non-executing frames are assumed to Remove a playerFrame (eep D:)
    //This is a valid assumption because max guy index of arrivals in a frame can be in one
    // of four categories prior to this being called:
    // maxGuyIndex == undefined, no guys:
    //					in this case the frame cannot contain a guy unless a guy arrives (which would trigger an update)
    // maxGuyIndex < playerInput_.size():
    //					in this case maxGuyIndex is guaranteed to be < playerInput.size() - 1 after this is called
    //					(as this increases size by 1) and so the maxGuyIndex guy is neither a currentPlayer nor nextPlayer)
    // maxGuyIndex == playerInput_.size():
    //					in this case maxGuyIndex == playerInput_.size() - 1 and so (barring new arrivals) 
    //					it contains a currentPlayer. However, in this case physics would have added this frame to nextPlayerFrames_
    //					and so it was added to be executed just above
    // maxGuyIndex > playerInput_.size():
    //					this can never happen, as physics cannot create a guy departure without having input for the guy
    //					and input does not exist unless maxGuyIndex < playerInput_.size()
    currentPlayerFramesAndDirections_.clear();
    nextPlayerFrames_.clear();
}

FrameID WorldState::getCurrentPlayerFrame() const
{
    assert(currentPlayerFramesAndDirections_.size() == 1);
    return currentPlayerFramesAndDirections_.begin()->first;
}

FrameID WorldState::getNextPlayerFrame() const
{
    assert(nextPlayerFrames_.size() == 1);
    return *(nextPlayerFrames_.begin());
}

TimeDirection WorldState::getCurrentPlayerDirection() const
{
    assert(currentPlayerFramesAndDirections_.size() == 1);
    return currentPlayerFramesAndDirections_.begin()->second;
}
}//namespace hg
