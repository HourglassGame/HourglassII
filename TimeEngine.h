#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H
#include <vector>

#include "PhysicsEngine.h"
#include "WorldState.h"
#include "TotalState.h"

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

    /**********************************************************************************************
     * Uses the input to progress the state of the level (the TimeEngine) and returns the arrivals
     * at the frame in which where the player guy has arrived but has not yet recieved input.
     * Throws a ParadoxException if a paradox occurs.
     * Preconditions: checkConsistencyAndPropogateLevel must have been called exactly once and returned true on *this
     */
    //Note- this interface may change as the requirements of the front end become better known,
    //but the basic idea should be the same
    const ObjectList getNextPlayerFrame(const InputList& newInputData);
	
private:
    WorldState executeFrameUpdateStackNoParadoxCheck(WorldState currentState, 
                                                     ::std::vector<unsigned int> frameUpdateStack) const;
    
    //runs the frame update stack until empty
    WorldState executeFrameUpdateStack(WorldState currentState, 
                                             ::std::vector<unsigned int> frameUpdateStack) const;
    
    TotalState getNthState(TotalState initialState, unsigned long n) const;
    
    /**********************************************************************************************
     * Gets all arrivals to `frame' using `currentState', applies physics to get departures, 
     * applies departures to `currentState'. All changed departures are added to frameUpdateStack.
     */
	void updateFrame(unsigned int frame, ::std::vector<unsigned int>& frameUpdateStack, WorldState& currentState) const;
    
    //state of world at end of last executed frame
    WorldState endOfFrameState;
    
    // stores all player input
    ::std::vector<InputList> playerInput; 
    
    //stores the physical properties of the world and uses them to turn arrivals into departures
    PhysicsEngine physics;
};
}

#endif //HG_TIME_ENGINE_H
