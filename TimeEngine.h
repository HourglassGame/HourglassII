#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "WorldState.h"
#include "FrameID.h"

#include <boost/tuple/tuple.hpp>

#include <vector>

namespace hg {
class InputList;
class ObjectList;
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

    typedef ::std::vector<FrameUpdateSet> FrameListList;
    /************************
     * Takes the new input data and uses that to update the state of the world and returns the current player frame
     * and a list of the frames which were updated in each propagation round. The current player frame is the last
     * in which the player had input.
     */
    ::boost::tuple<FrameID, FrameListList> runToNextPlayerFrame(const InputList& newInputData);
    /****************************
    * Returns an object list containing the state of whichFrame after physics was applied.
    * This function is always run after the runToNextPlayerFrame function in order to
    * query the state of particular frames.
    */
    ObjectList getPostPhysics(FrameID whichFrame) const;
private:
    //state of world at end of last executed frame
    WorldState worldState;
};
}

#endif //HG_TIME_ENGINE_H
