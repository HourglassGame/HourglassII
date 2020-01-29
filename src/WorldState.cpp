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
#include <iostream>

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
static std::vector<ConcurrentTimeSet> fixFrameVector(
    FramePointerUpdater const& frameUpdater,
    std::vector<ConcurrentTimeSet> const& oldVector)
{
    std::vector<ConcurrentTimeSet> newVector;
    for (unsigned int i = 0; i < oldVector.size(); ++i)
    {
        newVector.push_back({});
        for (hg::Frame *frame : oldVector[i]) {
            newVector[i].add(frame);
        }
    }
    return newVector;
}
WorldState::WorldState(WorldState const& o) :
        timeline_(o.timeline_),
        playerInput_(o.playerInput_),
        frameUpdateSet_(fixFrameUpdateSet(FramePointerUpdater(timeline_.getUniverse()), o.frameUpdateSet_)),
        physics_(o.physics_),
        guyNewArrivalFrames_(fixFrameVector(FramePointerUpdater(timeline_.getUniverse()), o.guyNewArrivalFrames_)),
        guyProcessedArrivalFrames_(fixFrameVector(FramePointerUpdater(timeline_.getUniverse()), o.guyProcessedArrivalFrames_)),
        processedGuyByFrame_(o.processedGuyByFrame_),
        currentWinFrames_(fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentWinFrames_))
{
}
WorldState &WorldState::operator=(WorldState const& o)
{
    timeline_ = o.timeline_;
    playerInput_ = o.playerInput_;
    frameUpdateSet_ = fixFrameUpdateSet(FramePointerUpdater(timeline_.getUniverse()), o.frameUpdateSet_);
    physics_ = o.physics_;
    guyNewArrivalFrames_ = fixFrameVector(FramePointerUpdater(timeline_.getUniverse()), o.guyNewArrivalFrames_);
    currentWinFrames_ = fixConcurrentTimeSet(FramePointerUpdater(timeline_.getUniverse()), o.currentWinFrames_);
    return *this;
}

WorldState::WorldState(
    int timelineLength,
    unsigned defaultSpeedOfTime,
    Guy const &initialGuy,
    FrameID const &guyStartTime,
    PhysicsEngine &&physics,
    ObjectList<NonGuyDynamic> &&initialObjects,
    OperationInterrupter &interrupter) :
        timeline_(timelineLength, defaultSpeedOfTime),
        playerInput_(),
        frameUpdateSet_(),
        physics_(std::move(physics)),
        guyNewArrivalFrames_(),
        guyProcessedArrivalFrames_(),
        processedGuyByFrame_(),
        currentWinFrames_()
{
    assert(guyStartTime.isValidFrame());
    assert(timelineLength > 0);

    Frame *guyStartFrame(timeline_.getFrame(guyStartTime));
    guyNewArrivalFrames_.push_back(ConcurrentTimeSet());
    guyNewArrivalFrames_.back().add(guyStartFrame);
    guyProcessedArrivalFrames_.push_back({}); // The oldest Guy to arrive has no input, but has no impact on the physics of the frame.

    for (int i = 0; i < timelineLength; ++i) {
        processedGuyByFrame_.push_back({});
    }

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
        executeWorld(interrupter, 0);
    }
}

void WorldState::swap(WorldState &o) noexcept
{
    boost::swap(timeline_, o.timeline_);
    boost::swap(playerInput_, o.playerInput_);
    boost::swap(frameUpdateSet_, o.frameUpdateSet_);
    boost::swap(physics_, o.physics_);
    boost::swap(guyNewArrivalFrames_, o.guyNewArrivalFrames_);
    boost::swap(guyProcessedArrivalFrames_, o.guyProcessedArrivalFrames_);
    boost::swap(processedGuyByFrame_, o.processedGuyByFrame_);
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
    WorldState::getDeparturesFromFrameAndUpdateSpeedOfTime(Frame *frame, OperationInterrupter &interrupter)
{
    ObjectPtrList<Normal> const arrivals = frame->getPrePhysics();
    std::vector<int> &processedGuys = processedGuyByFrame_[getFrameNumber(frame)];

    // Update the vector of guys that have arrived and been proccessed by this frame.
    for (std::size_t i(0), isize(processedGuys.size()); i < isize; ++i)
    {
        guyProcessedArrivalFrames_[processedGuys[i]].remove(frame);
    }

    processedGuys.resize(arrivals.getList<Guy>().size());
    int guyFrameIndex = 0;
    for (Guy const &guy : arrivals.getList<Guy>())
    {
        guyProcessedArrivalFrames_[guy.getIndex()].add(frame);
        processedGuys[guyFrameIndex] = guy.getIndex();
        ++guyFrameIndex;
    }

    // Run physics to turn the changed arrivals into departures.
    // executeFrame as it is supposed to have no side effects.
    PhysicsEngine::PhysicsReturnT retv(
        physics_.executeFrame(arrivals,
            frame,
            playerInput_,
            interrupter));

    for (unsigned i = 0; i < retv.guyDepartureFrames.size(); ++i)
    {
        guyNewArrivalFrames_[std::get<0>(retv.guyDepartureFrames[i])].add(std::get<1>(retv.guyDepartureFrames[i]));
    }
    if (retv.currentWinFrame) {
        currentWinFrames_.add(frame);
    }
    else {
        currentWinFrames_.remove(frame);
    }
    // retv.view should be removed and added to a function whose only purpose is to process
    // the arrivals for the timeline view. This would do frame->setView and also perform the
    // modification of guyProcessedArrivalFrames_.
    frame->setView(std::move(retv.view));

    // Update speed of time
    if (retv.speedOfTime >= 0) {
        frame->setSpeedOfTime(retv.speedOfTime);
    }

    return std::move(retv.departures);
}
FrameUpdateSet WorldState::executeWorld(OperationInterrupter &interrupter, unsigned executionCount)
{
    //std::cerr << "START: " << executionCount << "\n";
    DepartureMap newDepartures;
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    newDepartures.makeSpaceFor(frameUpdateSet_, executionCount);
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
            [&](Frame *frame) { 
                if (getFrameSpeedOfTime(frame) > executionCount) {
                    //std::cerr << "setDeparture: " << getFrameNumber(frame) << "\n";
                    newDepartures.setDeparture(frame, this->getDeparturesFromFrameAndUpdateSpeedOfTime(frame, interrupter));
                }
                else {
                    //std::cerr << "DoChange: " << getFrameNumber(frame) << "\n";
                    framesWithChangedArrivals.add(frame);
                }
            },
            group
        );
    }
    //Can `updateWithNewDepartures` take a long period of time?
    //If so, it needs to be given some way of being interrupted. (it needs to get passed the interrupter)
    frameUpdateSet_ = timeline_.updateWithNewDepartures(newDepartures, framesWithChangedArrivals);
    if (frameUpdateSet_.empty() && !currentWinFrames_.empty()) {
        assert(currentWinFrames_.size() == 1 
            && "How can a consistent reality have a guy win in multiple frames?");
        throw boost::enable_current_exception(PlayerVictoryException());
    }

    //std::cerr << "executionCount: " << executionCount << ", newDepartures: " << newDepartures.size() << ", returnSet: " << returnSet.size() << "\n";
    return returnSet;
}

//The basic assertion in the whole system is maxGuyIndex <= playerInput_.size()
/**
* Stores the given input data, allowing the player to exist for another step.
*/
void WorldState::addNewInputData(InputList const &newInputData)
{
    assert(newInputData.getRelativeGuyIndex() <= playerInput_.size());
    guyNewArrivalFrames_.push_back({});
    guyProcessedArrivalFrames_.push_back({});
    realPlayerInput_.push_back(newInputData);
    std::size_t const guyInputIndex = playerInput_.size() - newInputData.getRelativeGuyIndex();

    // Frame update always occurs for the oldest guy.
    for (hg::Frame *frame : guyNewArrivalFrames_[playerInput_.size()])
    {
        frameUpdateSet_.add(frame);
    }

    if (newInputData.getRelativeGuyIndex() > 0)
    {
        playerInput_.push_back(GuyInput());
        if (playerInput_[guyInputIndex] != newInputData.getGuyInput()) {
            playerInput_[guyInputIndex] = newInputData.getGuyInput();
            
            for (hg::Frame *frame : guyNewArrivalFrames_[guyInputIndex])
            {
                frameUpdateSet_.add(frame);
            }
        }
    }
    else
    {
        playerInput_.push_back(newInputData.getGuyInput());
    }
}

}//namespace hg
