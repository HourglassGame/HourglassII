#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include "TimelineState.h"
#include "FrameUpdateSet.h"
#include "FrameID.h"
#include "InputList.h"
#include "PhysicsEngine.h"
#include "FrameUpdateSet.h"

namespace hg {
    class WorldState {
    public:
        WorldState(const TimelineState& timeline,
                   unsigned int timelineLength,
                   FrameID guyStartTime,
                   PhysicsEngine physics,
                   const ObjectList& initialObjects);
        FrameUpdateSet executeWorld();
        ObjectList getPostPhysics(FrameID whichFrame) const;
        void addNewInputData(const InputList& newInputData);
        FrameID getCurrentPlayerFrame() const;
    private:
        TimeObjectListList getDeparturesFromFrame(const TimelineState::Frame& frame);
        
        TimelineState timeline_;
        //The frame constaining the guy with the largest overall relative index - who has arrived but not yet departed
        FrameID nextPlayerFrame_;
        //The frame containing the guy with the largest relative index who has both arrived and departed
        FrameID currentPlayerFrame_;
        // stores all player input
        ::std::vector<InputList> playerInput_;
        //Stores the frames which changed in the most recent call to executeWorld
        FrameUpdateSet frameUpdateSet_;
        //stores the physical properties of the world and uses them to turn arrivals into departures
        PhysicsEngine physics_;
    };
}
#endif //HG_WORLD_STATE_H
