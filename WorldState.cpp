#include "WorldState.h"

#include "DepartureMap.h"
#include "PlayerVictoryException.h"
#include "ParallelForEach.h"
#include "Frame.h"

#include <boost/foreach.hpp>

#define foreach BOOST_FOREACH

namespace hg {

struct ExecuteFrame
{
    ExecuteFrame(
        WorldState& thisptr,
        DepartureMap& departureMap):
            thisptr_(thisptr),
            departureMap_(departureMap)
    {
    }
    void operator()(Frame* time) const
    {
        //departures is moved into departureMap_, and so must be declared separately
        //(until r-value references become more widely supported)
        std::map<Frame*, ObjectList<Normal> > departures(thisptr_.getDeparturesFromFrame(time));
        departureMap_.setDeparture(time, departures);
    }
private:
    WorldState& thisptr_;
    DepartureMap& departureMap_;
};

WorldState::WorldState(
    std::size_t timelineLength,
    const FrameID& guyStartTime,
    const PhysicsEngine& physics,
    const ObjectList<Normal> & initialObjects) :
        timeline_(timelineLength),
        playerInput_(),
        frameUpdateSet_(),
        physics_(physics),
        nextPlayerFrames_(),
        currentPlayerFrames_(),
        currentWinFrames_()
{
    assert(guyStartTime.isValidFrame());
    Frame* guyStartFrame(timeline_.getFrame(guyStartTime));
    nextPlayerFrames_.add(guyStartFrame);
    {
        std::map<Frame*, ObjectList<Normal> > initialArrivals;

        // boxes
        for (std::vector<Box>::const_iterator it(initialObjects.getList<Box>().begin()),
                end(initialObjects.getList<Box>().end()); it != end; ++it)
        {
            initialArrivals[getEntryFrame(timeline_.getUniverse(), it->getTimeDirection())].add(*it);
        }
        
        // guy
        assert(initialObjects.getList<Guy>().size() == 1
               && "This should throw an exception rather than be an assert, but I can't be bothered right now");
        initialArrivals[guyStartFrame].add(initialObjects.getList<Guy>()[0]);

        timeline_.addArrivalsFromPermanentDepartureFrame(initialArrivals);
    }
    frameUpdateSet_.add(getEntryFrame(timeline_.getUniverse(), FORWARDS));
    frameUpdateSet_.add(getEntryFrame(timeline_.getUniverse(), REVERSE));
    //Guys without input can still affect stuff, and so must be run
    frameUpdateSet_.add(guyStartFrame);
    //** run level for a while
    for (std::size_t i(0); i < timelineLength; ++i) {
        executeWorld();
    }
}

Frame* WorldState::getFrame(const FrameID& whichFrame)
{
    return timeline_.getFrame(whichFrame);
}

std::map<Frame*, ObjectList<Normal> >
    WorldState::getDeparturesFromFrame(Frame* frame)
{
    PhysicsEngine::PhysicsReturnT retv(
        physics_.executeFrame(frame->getPrePhysics(),
                              frame,
                              playerInput_));
    if (retv.currentPlayerFrame) {
        currentPlayerFrames_.add(frame);
    }
    else {
        currentPlayerFrames_.remove(frame);
    }
    if (retv.nextPlayerFrame) {
        nextPlayerFrames_.add(frame);
    }
    else {
        nextPlayerFrames_.remove(frame);
    }
    if (retv.currentWinFrame) {
        currentWinFrames_.add(frame);
    }
    else {
        currentWinFrames_.remove(frame);
    }
    frame->setGlitzFromHere(retv.glitz);
    return retv.departures;
}

FrameUpdateSet WorldState::executeWorld()
{
    DepartureMap newDepartures;
    newDepartures.makeSpaceFor(frameUpdateSet_);
    FrameUpdateSet returnSet;
    frameUpdateSet_.swap(returnSet);
    parallel_for_each(returnSet, ExecuteFrame(*this, newDepartures));
    timeline_.updateWithNewDepartures(newDepartures).swap(frameUpdateSet_);
    if (frameUpdateSet_.empty() && !currentWinFrames_.empty()) {
        assert(currentWinFrames_.size() == 1 
            && "How can a consistent reality have a guy win in multiple frames?");
        throw PlayerVictoryException();
    }
    return returnSet;
}

//The worrying situation is when there are arrivals at a frame which are equal to arrivals which happened earlier,
//except that the effects of the new arrivals are different from the old ones because playerInput_.Count is larger
//than it was before. This situation breaks the wave assumption that departures == apply_physics(arrivals).

//this is ok because a new arrival with maxGuyIndex >= playerInput_.size() is either:
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
    for (ConcurrentTimeSet::iterator it(nextPlayerFrames_.begin()), end(nextPlayerFrames_.end()); it != end; ++it) {
        frameUpdateSet_.add(*it);
    }
    //All non-executing frames are assumed contain neither the currentPlayer nor the nextPlayer (eep D:)
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
    currentPlayerFrames_.clear();
    nextPlayerFrames_.clear();
}

Frame* WorldState::getCurrentPlayerFrame()
{
    if (!currentPlayerFrames_.empty()) {
        assert(currentPlayerFrames_.size() == 1);
        return currentPlayerFrames_.front();
    }
    else {
        return 0;
    }
}

Frame* WorldState::getNextPlayerFrame()
{
    if (!nextPlayerFrames_.empty()) {
        assert(nextPlayerFrames_.size() == 1);
        return *(nextPlayerFrames_.begin());
    }
    else {
        return 0;
    }
}
}//namespace hg
