#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "PhysicsEngine.h"
#include "WorldState.h"
#include "TimelineState.h"

#include <boost/tuple/tuple.hpp>

#include <vector>

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
               NewFrameID guyStartTime,
               AttachmentMap nAttachmentMap,
               TriggerSystem nTriggerSystem);

    typedef ::std::vector<FrameUpdateSet> FrameListList;
	/************************
     * Takes the new input data and uses that to update the state of the world and returns the current player frame
     * and a list of the frames which were updated in each propagation round. The current player frame is the last
     * in which the player had input.
     */
    ::boost::tuple<NewFrameID, FrameListList, TimeDirection> runToNextPlayerFrame(const InputList& newInputData);
	/****************************
    * Returns an object list containing the state of whichFrame after physics was applied.
    * This function is always run after the runToNextPlayerFrame function in order to
    * query the state of particular frames.
    */
    ObjectList getPostPhysics(NewFrameID whichFrame) const;
private:
    void executeWorld(WorldState& currentState) const;

    //state of world at end of last executed frame
    WorldState worldState;
};
}

#endif //HG_TIME_ENGINE_H
