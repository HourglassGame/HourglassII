#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H
#include <vector>

#include <boost/tuple/tuple.hpp>

#include "PhysicsEngine.h"
#include "WorldState.h"
#include "TimelineState.h"

#include "FrameID.h"

namespace hg {
class InputList;
class ObjectList;
class TimeObjectListList;
class ParadoxException;

class TimeEngine
{
public:
    /*********************************************************************************************
     * Constructs a new TimeEngine with the given timeline length, wall, wall size and gravity
     * wall size is the length along one of the sides of the square wall segments given by wallmap
     *
     * Propogates the level with the given ObjectList to fully initialise the TimeEngine 
     * (objects begin at all points in time throughout the level,
     * and so must be propogated through from the start and the end)
     * Throws InvalidLevelException if level is not correct and consistent.
     * A correct level has exacty one guy.
     * A consistent level has a state which does not depend on the direction in which it is propogated
     */
	TimeEngine(unsigned int timeLineLength,
               ::std::vector< ::std::vector<bool> > wallmap,
               int newWallSize,
               int newGravity, 
               const ObjectList& initialObjects,
               unsigned int guyStartTime);

    typedef ::std::vector< ::std::vector<FrameID> > FrameListList;
    ::boost::tuple<FrameID, FrameListList> runToNextPlayerFrame(const InputList& newInputData);
    ObjectList getPostPhysics(FrameID whichFrame) const;
private:
    void executeWorld(WorldState& currentState) const;
    TimeObjectListList getDeparturesFromFrame(const TimelineState::Frame& frame, FrameID& currentPlayerFrame, FrameID& nextPlayerFrame) const;
    
    //state of world at end of last executed frame
    WorldState endOfFrameState;
    
    // stores all player input
    ::std::vector<InputList> playerInput; 
    
    //stores the physical properties of the world and uses them to turn arrivals into departures
    PhysicsEngine physics;
};
}

#endif //HG_TIME_ENGINE_H
