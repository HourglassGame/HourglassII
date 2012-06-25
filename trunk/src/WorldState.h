#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H

#include "TimelineState.h"
#include "FrameUpdateSet.h"
#include "PhysicsEngine.h"
#include "FrameUpdateSet.h"
#include "ConcurrentTimeSet.h"
#include "InputList.h"

#include <tbb/task.h>

#include <vector>
#include <boost/move/move.hpp>

#include "Frame_fwd.h"
#include "FrameID_fwd.h"
namespace hg {
struct ExecuteFrame;
struct NewExecuteFrame;
struct EditDepartures;

class WorldState {
public:
    /**
     * Creates a new world state.
     * Throws an exception if the world state is not consistent.
     */
    //Exception Safety: Strong
    WorldState(
        std::size_t timelineLength,
        Guy const& initialGuy,
        FrameID const& guyStartTime,
        BOOST_RV_REF(PhysicsEngine) physics,
        BOOST_RV_REF(ObjectList<NonGuyDynamic>) initialObjects,
        OperationInterrupter& interrupter);

    void swap(WorldState& o);

    WorldState(BOOST_RV_REF(WorldState) o) :
        timeline_(boost::move(o.timeline_)),
        playerInput_(boost::move(o.playerInput_)),
        frameUpdateSet_(boost::move(o.frameUpdateSet_)),
        physics_(boost::move(o.physics_)),
        nextPlayerFrames_(boost::move(o.nextPlayerFrames_)),
        currentPlayerFrames_(boost::move(o.currentPlayerFrames_)),
        currentWinFrames_(boost::move(o.currentWinFrames_))
    {}
     
    WorldState& operator=(BOOST_RV_REF(WorldState) o)
    {
        timeline_ = boost::move(o.timeline_);
        playerInput_ = boost::move(o.playerInput_);
        frameUpdateSet_ = boost::move(o.frameUpdateSet_);
        physics_ = boost::move(o.physics_);
        nextPlayerFrames_ = boost::move(o.nextPlayerFrames_);
        currentPlayerFrames_ = boost::move(o.currentPlayerFrames_);
        currentWinFrames_ = boost::move(o.currentWinFrames_);
        return *this;
    }

    /**
     * Updates the state of the world once.
     * Throws PlayerVictoryException if the player has won
     * in exactly one frame and there are no waves.
     * Returns the set of frames that were executed (had different arrivals).
     */
    FrameUpdateSet executeWorld(OperationInterrupter& interrupter);

    /**
     * Stores the given input data, allowing the player to exist for another step.
     */
    void addNewInputData(InputList const& newInputData);

    /**
    * Returns the frame containing the oldest (highest relative index) Guy who has input.
    */
    Frame* getNextPlayerFrame();
    Frame* getCurrentPlayerFrame();

    std::vector<InputList> const& getReplayData() const { return playerInput_; }

    Frame* getFrame(FrameID const& whichFrame);
    
    std::size_t getTimelineLength() const;

private:
    friend struct ExecuteFrame;
    PhysicsEngine::FrameDepartureT
        getDeparturesFromFrame(Frame* frame, OperationInterrupter& interrupter);
    
    TimelineState timeline_;
    //Stores all player input (go left/right, jump, etc...). Each element in the vector corresponds to
    //the input for the guy with the index corresponding to that element.
    std::vector<InputList> playerInput_;
    //Stores the frames whose arrivals have changed but which have not been executed since the change.
    //executeWorld executes every frame in frameUpdateSet_
    FrameUpdateSet frameUpdateSet_;
    //Stores the physical properties of the world and uses them to turn arrivals into departures.
    PhysicsEngine physics_;
    //The set of frames containing the guys with the largest overall relative index - who has arrived but not yet departed.
    //The important thing about nextPlayerFrames_ is that it must hold at least the set of frames that will have their 
    //departures changed by the addition of new player input without the changing of arrivals.
    ConcurrentTimeSet nextPlayerFrames_;
    //The frame containing the guy with the largest relative index who has arrived and has input.
    ConcurrentTimeSet currentPlayerFrames_;
    //Holds the frame(s) in which the win condition is met in the current universe state.
    //That is - just the frame(s) when a guy actually went through the end portal 
    //(or whatever the win condition is) and not the following frames when the
    //win condition has been met at some previous time.
    ConcurrentTimeSet currentWinFrames_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE(WorldState)
};
inline void swap(WorldState& l, WorldState& r) { l.swap(r); }
}
#endif //HG_WORLD_STATE_H