#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H
#include "PhysicsEngine.h"
#include "WorldState.h"

#include <vector>

namespace hg {
class InputList;
class ObjectList;
class TimeObjectListList;
class ParadoxException;
class WorldState;
    
class TimeEngine
{

public:
	TimeEngine(int newTimeLength, ::std::vector< ::std::vector<bool> > wallmap, int newWallSize, int newGravity);
    
    // returns if the level creator not a foo
    // Call before trying to use level
	bool checkConstistencyAndPropagateLevel(const ObjectList& initialObjects, int guyStartTime);
    

    ObjectList getNextPlayerFrame(const InputList& newInputData);
	
private:
    WorldState executeFrameUpdateStackNoParadoxCheck(WorldState currentState, 
                                                     ::std::vector<int> frameUpdateStack) const;
    
    //runs the frame update stack until empty
    WorldState executeFrameUpdateStack(WorldState currentState, 
                                             ::std::vector<int> frameUpdateStack) const;
    
    //Gets all arrivals to `frame' using `currentState', applies physics to get departures, 
    //applies departures to `currentState'. All changed departures are added to frameUpdateStack.
	void updateFrame(int frame, ::std::vector<int>& frameUpdateStack, WorldState& currentState) const;
    
    //state of world at end of last executed frame
    WorldState endOfFrameState;
    
	int timeLineLength; // size of playable timeline

    ::std::vector<InputList> playerInput; // stores all player input

    //stores the physical properties of the world and uses them to turn arrivals into departures
    PhysicsEngine physics;
};
}

#endif //HG_TIME_ENGINE_H
