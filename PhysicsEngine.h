#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "ObjectList.h"
#include "InputList.h"
#include "TimeObjectListList.h"
#include <vector>
namespace hg {
class PhysicsEngine
{
public:

	PhysicsEngine(int newTimeLineLength, 
                  std::vector<std::vector<bool> > newWallmap,
                  int newWallSize, 
                  int newGravity);

    
    // executes frame and returns departures
	hg::TimeObjectListList executeFrame(const hg::ObjectList& arrivals,
                                        int time,
                                        const std::vector<hg::InputList>& playerInput,
                                        //Filled with the frame which the player departs for 
                                        //or arrives at if appropriate, otherwise untouched.
                                        int& currentPlayerFrame,
                                        int& nextPlayerFrame) const;
    
private:
    struct BoxInfo {
        BoxInfo(hg::Box nbox,
                bool nsupported) :
        box(nbox),
        supported(nsupported)
        {}
        hg::Box box;
        bool supported;
    };
	void crappyBoxCollisionAlogorithm(const std::vector<hg::Box>& oldBoxList,
                                      std::vector<PhysicsEngine::BoxInfo>& nextBox) const;
	
    void guyStep(const std::vector<Guy>& oldGuyList, int time, 
                 const std::vector<InputList>& playerInput, TimeObjectListList& newDepartures,
                 std::vector<PhysicsEngine::BoxInfo>& nextBox,
                 int& currentPlayerFrame,
                 int& nextPlayerFrame) const;

	bool wallAt(int x, int y) const;
    
    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    int timeLineLength;
    std::vector<std::vector<bool> > wallmap;
	int gravity;
	int wallSize;
};
}
#endif //HG_PHYSICS_ENGINE_H
