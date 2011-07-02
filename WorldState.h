#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H

#include "TimelineState.h"
#include "FrameUpdateSet.h"
#include "PhysicsEngine.h"
#include "FrameUpdateSet.h"
#include "ConcurrentTimeSet.h"
#include "InputList.h"

#include <vector>

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
    WorldState(
        std::size_t timelineLength,
        FrameID const& guyStartTime,
        PhysicsEngine const& physics,
        ObjectList<Normal> const& initialObjects);

    /**
     * Updates the state of the world once.
     * Throws PlayerVictoryException if the player has won
     * in exactly one frame and there are no waves.
     * Returns the set of frames that were executed (had different arrivals).
     * Return valuse does not include those frames that just had their departures edited.
     */
    FrameUpdateSet executeWorld();

    /**
     * Stores the given input data, allowing the player to exist for another step.
     */
    void addNewInputData(InputList const& newInputData);

    /**
    * Returns the frame containing the oldest (highest relative index) Guy who has input.
    */
    Frame* getNextPlayerFrame();
    Frame* getCurrentPlayerFrame();

    std::vector<InputList> getReplayData() const { return playerInput_; }

    Frame* getFrame(FrameID const& whichFrame);
private:
    friend struct ExecuteFrame;
    friend struct NewExecuteFrame;
    friend struct EditDepartures;
    std::pair<
        std::map<Frame*, ObjectList<Normal> >,
        std::map<Frame*, ObjectList<FirstEdit> > >
    getDeparturesFromFrame(Frame* frame);
    
    std::map<Frame*, ObjectList<Normal> > getEditedDeparturesFromFrame(Frame const* frame);
    TimelineState timeline_;
    //Stores all player input (go left/right, jump, etc...). Each element in the vector corresponds to
    //the input for the guy with the index corresponding to that element.
    std::vector<InputList> playerInput_;
    //Stores the frames whose arrivals have changed but which have not been executed since the change.
    //executeWorld executes every frame in frameUpdateSet_
    FrameUpdateSet frameUpdateSet_;
    //Stores the physical properties of the world and uses them to turn arrivals into departures.
    PhysicsEngine physics_;
    //The frame containing the guy with the largest overall relative index - who has arrived but not yet departed.
    ConcurrentTimeSet nextPlayerFrames_;
    //The frame containing the guy with the largest relative index who has both arrived and departed.
    ConcurrentTimeSet currentPlayerFrames_;
    //Holds the frame(s) in which the win condition is met in the current universe state.
    //That is - just the frame(s) when a guy actually went through the end portal 
    //(or whatever the win condition is) and not the following frames when the
    //win condition has been met at some previous time.
    ConcurrentTimeSet currentWinFrames_;
};
}
#endif //HG_WORLD_STATE_H
