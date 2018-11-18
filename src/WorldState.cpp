#include "WorldState.h"

#include "DepartureMap.h"
#include "PlayerVictoryException.h"
#include "ParallelForEach.h"
#include "Frame.h"
#include "Universe.h"

#include "OperationInterruptedException.h"

#include <utility>
#include <tbb/task_group.h>
#include <thread>

namespace hg {
static FrameUpdateSet fixFrameUpdateSet(FramePointerUpdater const& frameUpdater, FrameUpdateSet const& oldFrameUpdateSet)
{
    FrameUpdateSet newFrameUpdateSet;
    for (auto frame: oldFrameUpdateSet) {
        newFrameUpdateSet.add(frameUpdater.updateFrame(frame));
    }
    newFrameUpdateSet.make_set();
    return newFrameUpdateSet;
}
static ConcurrentTimeSet fixConcurrentTimeSet(
    FramePointerUpdater const& frameUpdater,
    ConcurrentTimeSet const& oldSet)
{
    ConcurrentTimeSet newSet;
    for (auto frame: oldSet) {
        newSet.add(frameUpdater.updateFrame(frame));
    }
    return newSet;
}
static std::vector<Frame*> fixFrameVector(
    FramePointerUpdater const& frameUpdater,
    std::vector<Frame*> const& oldVector)
{
    std::vector<Frame*> newVector;
    for (unsigned int i = 0; i < oldVector.size(); ++i)
    {
        newVector.push_back(frameUpdater.updateFrame(oldVector[i]));
    }
    return newVector;
}
WorldState::WorldState(WorldState const& o) :
        timeline_(o.timeline_),
        playerInput_(o.playerInput_),
        frameUpdateSet_(fixFrameUpdateSet(FramePointerUpdater(timeline_.getUniverse()), o.frameUpdateSet_)),
        physics_(o.physics_),
        guyArrivalFrames_(fixFrameVector(FramePointerUpdater(timeline_.getUniverse()), o.guyArrivalFrames_)),
        nextPlayerFrames_(fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.nextPlayerFrames_)),
        currentPlayerFrames_(fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentPlayerFrames_)),
        currentWinFrames_(fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentWinFrames_))
{
}
WorldState &WorldState::operator=(WorldState const& o)
{
    timeline_ = o.timeline_;
    playerInput_ = o.playerInput_;
    frameUpdateSet_ = fixFrameUpdateSet(FramePointerUpdater(timeline_.getUniverse()), o.frameUpdateSet_);
    physics_ = o.physics_;
    guyArrivalFrames_ = fixFrameVector(FramePointerUpdater(timeline_.getUniverse()), o.guyArrivalFrames_);
    nextPlayerFrames_ = fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.nextPlayerFrames_);
    currentPlayerFrames_ = fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentPlayerFrames_);
    currentWinFrames_ = fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentWinFrames_);
    return *this;
}

WorldState::WorldState(
    int timelineLength,
    Guy const &initialGuy,
    FrameID const &guyStartTime,
    PhysicsEngine &&physics,
    ObjectList<NonGuyDynamic> &&initialObjects,
    OperationInterrupter &interrupter) :
        timeline_(timelineLength),
        playerInput_(),
        frameUpdateSet_(),
        physics_(std::move(physics)),
        nextPlayerFrames_(),
        currentPlayerFrames_(),
        currentWinFrames_()
{
    assert(guyStartTime.isValidFrame());
    assert(timelineLength > 0);

    Frame *guyStartFrame(timeline_.getFrame(guyStartTime));
    guyArrivalFrames_.push_back(guyStartFrame);

    nextPlayerFrames_.add(guyStartFrame);
    {
        std::map<Frame *, ObjectList<Normal>> initialArrivals;

        // boxes
        for (Box const &box: initialObjects.getList<Box>())
        {
            initialArrivals[getEntryFrame(timeline_.getUniverse(), box.getTimeDirection())].add(box);
        }
        
        // guy
        assert(initialGuy.getIndex() == 0);
        initialArrivals[guyStartFrame].add(initialGuy);

        timeline_.addArrivalsFromPermanentDepartureFrame(initialArrivals);
    }
    //triggerSystem can create departures, so every frame must be initially run:
    for (int i(0); i != timelineLength; ++i) {
        frameUpdateSet_.add(getArbitraryFrame(timeline_.getUniverse(), i));
    }
    //TODO: Give some way for the UI to hook into this, for a 'Debug' loading view...
    //Run level for a while
    for (int i(0); i != timelineLength && !interrupter.interrupted(); ++i) {
        executeWorld(interrupter);
    }
}

void WorldState::swap(WorldState &o) noexcept
{
    boost::swap(timeline_, o.timeline_);
    boost::swap(playerInput_, o.playerInput_);
    boost::swap(frameUpdateSet_, o.frameUpdateSet_);
    boost::swap(physics_, o.physics_);
    boost::swap(nextPlayerFrames_, o.nextPlayerFrames_);
    boost::swap(currentPlayerFrames_, o.currentPlayerFrames_);
    boost::swap(currentWinFrames_, o.currentWinFrames_);
}

Frame const *WorldState::getFrame(FrameID const &whichFrame) const
{
    return timeline_.getFrame(whichFrame);
}

int WorldState::getTimelineLength() const
{
    return hg::getTimelineLength(timeline_.getUniverse());
}

PhysicsEngine::FrameDepartureT
    WorldState::getDeparturesFromFrame(Frame *frame, OperationInterrupter &interrupter)
{
    PhysicsEngine::PhysicsReturnT retv(
        physics_.executeFrame(frame->getPrePhysics(),
                              frame,
                              playerInput_,
                              interrupter));

    for (unsigned i = 0; i < retv.guyDepartureFrames.size(); ++i)
    {
        guyArrivalFrames_[std::get<0>(retv.guyDepartureFrames[i])] = std::get<1>(retv.guyDepartureFrames[i]);
    }
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
    frame->setView(std::move(retv.view));
    return std::move(retv.departures);
}
FrameUpdateSet WorldState::executeWorld(OperationInterrupter &interrupter)
{
    DepartureMap newDepartures;
    newDepartures.makeSpaceFor(frameUpdateSet_);
    FrameUpdateSet returnSet;
    frameUpdateSet_.swap(returnSet);
    {
        tbb::task_group_context group;
        auto group_interruption_scope =
            interrupter.addInterruptionFunction(
                [&]{
                    //This was previously implemented via group.register_pending_exception()
                    //Previously there was a comment here warning:
                    //'''
                    //Do not directly call `group.cancel_group_execution()`, as this can lead
                    //to a failure to propagate exceptions from ExecuteFrame, which can lead to
                    //the world continuing to be run while in an invalid state.
                    //'''
                    //I was unable to figure out why this was considered a problem; but maybe this comment can be a clue
                    //if the problem reapppears. (If no problem reappears, or you prove the non-existence of any problem,
                    //remove this comment.)
                    //Note that `group.register_pending_exception()` seems to be buggy in MSVC++, and will sometimes
                    //just throw the pending exception, rather than capturing it. (It is also an undocumented feature).
                    group.cancel_group_execution();
                  });
        parallel_for_each(
            returnSet,
            [&](Frame *frame) { newDepartures.setDeparture(frame, this->getDeparturesFromFrame(frame, interrupter)); },
            group);
    }
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
//            run, in which case nextPlayerFrames_ will be set and the frame will be run again when new input becomes available
//        or
//            not run, in which case it will be still sitting in frameUpdateSet_ and will be run in the next executeWorld
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
void WorldState::addNewInputData(InputList const &newInputData)
{
    assert(newInputData.getRelativeGuyIndex() > playerInput_.rbegin()); // Do not send input to negative guys.
    guyArrivalFrames_.push_back(nullptr);
    realPlayerInput_.push_back(newInputData);
    std::size_t guyInputIndex = playerInput_.size() - newInputData.getRelativeGuyIndex();
    if (newInputData.getRelativeGuyIndex() > 0)
    {
        playerInput_.push_back(GuyInput());
        playerInput_[guyInputIndex] = newInputData.getGuyInput();
        frameUpdateSet_.add(guyArrivalFrames_[guyInputIndex]); // Frame update is already occuring for the oldest guy.
    }
    else
    {
        playerInput_.push_back(newInputData.getGuyInput());
    }
    
    //All non-executing frames are assumed contain neither the currentPlayer nor the nextPlayer (eep D:)
    //This is a valid assumption because max guy index of arrivals in a frame can be in one
    // of four categories prior to this being called:
    // maxGuyIndex == undefined, no guys:
    //                    in this case the frame cannot contain a guy unless a guy arrives (which would trigger an update)
    // maxGuyIndex < playerInput_.size():
    //                    in this case maxGuyIndex is guaranteed to be < playerInput.size() - 1 after this is called
    //                    (as this increases size by 1) and so the maxGuyIndex guy is neither a currentPlayer nor nextPlayer)
    // maxGuyIndex == playerInput_.size():
    //                    in this case maxGuyIndex == playerInput_.size() - 1 and so (barring new arrivals)
    //                    it contains a currentPlayer. However, in this case physics would have added this frame to nextPlayerFrames_
    //                    and so it was added to be executed just above
    // maxGuyIndex > playerInput_.size():
    //                    this can never happen, as physics cannot create a guy departure without having input for the guy
    //                    and input does not exist unless maxGuyIndex < playerInput_.size()
    currentPlayerFrames_.clear();
    nextPlayerFrames_.clear();
}

Frame *WorldState::getCurrentPlayerFrame()
{
    if (!currentPlayerFrames_.empty()) {
        assert(currentPlayerFrames_.size() == 1);
        return currentPlayerFrames_.front();
    }
    else {
        return nullptr;
    }
}

Frame *WorldState::getNextPlayerFrame()
{
    if (!nextPlayerFrames_.empty()) {
        assert(nextPlayerFrames_.size() == 1);
        return nextPlayerFrames_.front();
    }
    else {
        return nullptr;
    }
}
}//namespace hg
