#include "WorldState.h"

#include "DepartureMap.h"
#include "PlayerVictoryException.h"
#include "ParallelForEach.h"
#include "Frame.h"
#include "Universe.h"

#include "Foreach.h"

#include <utility>

namespace hg {
struct ExecuteFrame
{
    ExecuteFrame(WorldState& worldState, DepartureMap& newDepartures, OperationInterrupter& interrupter) :
        worldState_(&worldState), newDepartures_(&newDepartures), interrupter_(&interrupter)
    {}
    void operator()(Frame* frame) const {
        newDepartures_->setDeparture(frame, worldState_->getDeparturesFromFrame(frame, *interrupter_));
    }
private:
    WorldState* worldState_;
    DepartureMap* newDepartures_;
    OperationInterrupter* interrupter_;
};

WorldState::WorldState(
    std::size_t timelineLength,
    Guy const& initialGuy,
    FrameID const& guyStartTime,
    BOOST_RV_REF(PhysicsEngine) physics,
    BOOST_RV_REF(ObjectList<NonGuyDynamic>) initialObjects,
    OperationInterrupter& interrupter) :
        timeline_(timelineLength),
        playerInput_(),
        frameUpdateSet_(),
        physics_(boost::move(physics)),
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
        foreach (Box const& box, initialObjects.getList<Box>())
        {
            initialArrivals[getEntryFrame(timeline_.getUniverse(), box.getTimeDirection())].add(box);
        }
        
        // guy
        assert(initialGuy.getIndex() == 0);
        initialArrivals[guyStartFrame].add(initialGuy);

        timeline_.addArrivalsFromPermanentDepartureFrame(initialArrivals);
    }
    //triggerSystem can create departures, so every frame must be initially run:
    for (std::size_t i(0); i != timelineLength; ++i) {
        frameUpdateSet_.add(getArbitraryFrame(timeline_.getUniverse(), i));
    }
    //Run level for a while
    for (std::size_t i(0); i != timelineLength; ++i) {
        executeWorld(interrupter);
    }
}

void WorldState::swap(WorldState& o)
{
    boost::swap(timeline_, o.timeline_);
    boost::swap(playerInput_, o.playerInput_);
    boost::swap(frameUpdateSet_, o.frameUpdateSet_);
    boost::swap(physics_, o.physics_);
    boost::swap(nextPlayerFrames_, o.nextPlayerFrames_);
    boost::swap(currentPlayerFrames_, o.currentPlayerFrames_);
    boost::swap(currentWinFrames_, o.currentWinFrames_);
}

Frame* WorldState::getFrame(FrameID const& whichFrame)
{
    return timeline_.getFrame(whichFrame);
}

std::size_t WorldState::getTimelineLength() const
{
    return hg::getTimelineLength(timeline_.getUniverse());
}

PhysicsEngine::FrameDepartureT
    WorldState::getDeparturesFromFrame(Frame* frame, OperationInterrupter& interrupter)
{
    PhysicsEngine::PhysicsReturnT retv(
        physics_.executeFrame(frame->getPrePhysics(),
                              frame,
                              playerInput_,
                              interrupter));
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
    frame->setView(retv.view);
    return retv.departures;
}

struct InterruptTaskGroup {
public:
    InterruptTaskGroup(tbb::task_group_context& task_group_context):
        task_group_context_(&task_group_context) {}
    void operator()() const {
        task_group_context_->cancel_group_execution();
    }
private:
    tbb::task_group_context* task_group_context_;
};

FrameUpdateSet WorldState::executeWorld(OperationInterrupter& interrupter)
{
    std::cerr << "Executing World\n";
    DepartureMap newDepartures;
    newDepartures.makeSpaceFor(frameUpdateSet_);
    FrameUpdateSet returnSet;
    frameUpdateSet_.swap(returnSet);
    tbb::task_group_context group;
    OperationInterrupter::FunctionHandle task_group_interrupt(
        interrupter.addInterruptionFunction(move_function<void()>(InterruptTaskGroup(group))));
    parallel_for_each(returnSet, ExecuteFrame(*this, newDepartures, interrupter), group);
    //Can `updateWithNewDepartures` take a long period of time?
    //If so, it needs to be given some way of being interrupted. (it needs to get passed the interrupter)
    frameUpdateSet_ = timeline_.updateWithNewDepartures(newDepartures);
    if (frameUpdateSet_.empty() && !currentWinFrames_.empty()) {
        assert(currentWinFrames_.size() == 1 
            && "How can a consistent reality have a guy win in multiple frames?");
        throw boost::enable_current_exception(PlayerVictoryException());
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
    foreach (Frame* frame, nextPlayerFrames_) {
        frameUpdateSet_.add(frame);
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
        return *nextPlayerFrames_.begin();
    }
    else {
        return 0;
    }
}
}//namespace hg
