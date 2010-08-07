#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H
#include "PhysicsEngine.h"
#include "TimeObjectListList.h"

#include "InputList.h"

#include "ArrivalDepartureMap.h"
#include "WorldState.h"

#include <boost/ptr_container/ptr_map.hpp>
#include <vector>
#include <stack>
namespace hg {
class TimeEngine
{

public:
	TimeEngine(int newTimeLength, std::vector<std::vector<bool> > wallmap, int newWallSize, int newGravity);
    // returns if the level creator not a foo
    // Call before trying to use level
	bool checkConstistencyAndPropagateLevel(const hg::ObjectList& initialObjects, int guyStartTime);

    hg::ObjectList getNextPlayerFrame(const hg::InputList& newInputData);
	
private:
    // runs the frame update stack until empty
    hg::WorldState executeFrameUpdateStack(WorldState currentState, 
                                                std::vector<int> frameUpdateStack) const;
    
    // updates the frame using new arrivals. departures are checked and added to frameUpdateStack if different
	void updateFrame(int frame, std::vector<int>& frameUpdateStack, WorldState& currentState) const;
    
    //state of world at end of last executed frame
    hg::WorldState endOfFrameState;
    
	int timeLineLength; // size of playable timeline

    std::vector<hg::InputList> playerInput; // stores all player input

    //stores the physical properties of the world and uses them to turn arrivals into departures
    hg::PhysicsEngine physics;
};
}
#endif //HG_TIME_ENGINE_H
