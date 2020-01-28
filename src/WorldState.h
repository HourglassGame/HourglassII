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
#include <utility>

#include "Frame_fwd.h"
#include "FrameID_fwd.h"
namespace hg {
struct NewExecuteFrame;
struct EditDepartures;

class WorldState final {
public:
    /**
     * Creates a new world state.
     * Throws an exception if the world state is not consistent.
     */
    //Exception Safety: Strong
    WorldState(
        int timelineLength,
        unsigned defaultSpeedOfTime,
        Guy const &initialGuy,
        FrameID const &guyStartTime,
        PhysicsEngine&& physics,
        ObjectList<NonGuyDynamic>&& initialObjects,
        OperationInterrupter &interrupter);

    void swap(WorldState &o) noexcept;

    WorldState(WorldState const& o);
    WorldState &operator=(WorldState const& o);

    WorldState(WorldState &&o) noexcept = default;
    WorldState &operator=(WorldState &&o) noexcept = default;
    /**
     * Updates the state of the world once.
     * Throws PlayerVictoryException if the player has won
     * in exactly one frame and there are no waves.
     * Returns the set of frames that were executed (had different arrivals).
     */
    FrameUpdateSet executeWorld(OperationInterrupter &interrupter, unsigned executionCount);

    /**
     * Stores the given input data, allowing the player to exist for another step.
     */
    void addNewInputData(InputList const &newInputData);

    /**
    * Returns the array mapping guy index to arrival frame.
    */
    std::vector<Frame *> const &getGuyFrames() const { return guyProcessedArrivalFrames_; };

    std::vector<GuyInput> const &getPostOverwriteInput() const { return playerInput_; }
    std::vector<InputList> const &getReplayData() const { return realPlayerInput_; }

    Frame const *getFrame(FrameID const &whichFrame) const;
    
    int getTimelineLength() const;

private:
    PhysicsEngine::FrameDepartureT
        getDeparturesFromFrameAndUpdateSpeedOfTime(Frame *frame, OperationInterrupter &interrupter);
    
    TimelineState timeline_;
    //Stores all player input (go left/right, jump, etc...). Each element in the vector corresponds to
    //the input for the guy with the index corresponding to that element.
    std::vector<GuyInput> playerInput_;
    std::vector<InputList> realPlayerInput_;
    //Stores the frames whose arrivals have changed but which have not been executed since the change.
    //executeWorld executes every frame in frameUpdateSet_
    FrameUpdateSet frameUpdateSet_;
    //Stores the physical properties of the world and uses them to turn arrivals into departures.
    PhysicsEngine physics_;
    //An array indexed by index of the frame at which a guy is known to arrive, but not necessarily processed by physics yet.
    std::vector<Frame*> guyNewArrivalFrames_;
    //An array indexed by index of the frame at which the guy with that index has arrived and been processed by physics.
    std::vector<Frame*> guyProcessedArrivalFrames_;
    //Holds the frame(s) in which the win condition is met in the current universe state.
    //That is - just the frame(s) when a guy actually went through the end portal 
    //(or whatever the win condition is) and not the following frames when the
    //win condition has been met at some previous time.
    ConcurrentTimeSet currentWinFrames_;
};
inline void swap(WorldState &l, WorldState &r) noexcept { l.swap(r); }
}
#endif //HG_WORLD_STATE_H
