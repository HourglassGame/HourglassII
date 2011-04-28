#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H

#include "TimelineState.h"
#include "FrameUpdateSet.h"
#include "InputList.h"
#include "PhysicsEngine.h"
#include "FrameUpdateSet.h"
#include "ConcurrentTimeSet.h"
#include "ConcurrentTimeMap.h"
#include <vector>

namespace hg {
    struct ExecuteFrame;
    class Frame;
    class FrameID;
    class WorldState {
    public:
        /******************
         * Creates a new world state.
         * Throws an exception if the world state is not consistent.
         */
        WorldState(std::size_t timelineLength,
                   FrameID guyStartTime,
                   const PhysicsEngine& physics,
                   const ObjectList& initialObjects);

        /***************************************
         * Updates the state of the world once.
         * throws PlayerVictoryException if the player has won
         * in exactly one frame and there are no waves.
         */
        FrameUpdateSet executeWorld();

        /*******************************
         * Stores the given input data, allowing the player to exist for another step.
         */
        void addNewInputData(const InputList& newInputData);

        /******************************************************************************
        * Returns an object list containing the state of whichFrame after physics was applied
        * in the last call to executeWorld.
        */
        //ObjectList getPostPhysics(Frame* whichFrame, const PauseInitiatorID& whichPrePause) const;
        /***********************************************************
        * Returns the frame containing the oldest (highest relative index) Guy who has input.
        */
        Frame* getNextPlayerFrame();
        Frame* getCurrentPlayerFrame();
        TimeDirection getCurrentPlayerDirection() const;
        
        std::vector<InputList> getReplayData() const {return playerInput_;};
        
        Frame* getFrame(const FrameID& whichFrame);
    private:
        friend struct ExecuteFrame;
        std::map<Frame*, ObjectList> getDeparturesFromFrame(Frame* frame);

        TimelineState timeline_;
        // stores all player input
        std::vector<InputList> playerInput_;
        //Stores the frames whose arrivals have changed but which have not been executed since the change
        //executeWorld executes every frame in frameUpdateSet_
        FrameUpdateSet frameUpdateSet_;
        //stores the physical properties of the world and uses them to turn arrivals into departures
        PhysicsEngine physics_;
        //The frame constaining the guy with the largest overall relative index - who has arrived but not yet departed
        ConcurrentTimeSet nextPlayerFrames_;
        //The frame containing the guy with the largest relative index who has both arrived and departed
        ConcurrentTimeMap currentPlayerFramesAndDirections_;
        //holds the frame in which the player won, or nullFrame if the player has not won at all.
        ConcurrentTimeSet currentWinFrames_;
    };
}
#endif //HG_WORLD_STATE_H
