#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "InputList.h"
#include "Guy.h"
#include "Box.h"
#include "FrameID.h"
#include "ObjectList.h"
#include <vector>
#include <map>
namespace hg {
class ObjectList;
class TimeObjectListList;
class PhysicsEngine
{
public:
	PhysicsEngine(unsigned int newTimeLineLength, 
                  ::std::vector< ::std::vector<bool> > newWallmap,
                  int newWallSize, 
                  int newGravity);
    
    // executes frame and returns departures
	TimeObjectListList executeFrame(const ObjectList& arrivals,
                                        FrameID time,
                                        const ::std::vector<InputList>& playerInput,
                                        FrameID& currentPlayerFrame,
                                        FrameID& nextPlayerFrame) const;
    
private:
    struct BoxInfo {
        BoxInfo(Box nbox,
                bool nsupported) :
        box(nbox),
        supported(nsupported)
        {}
        Box box;
        bool supported;
    };
	void crappyBoxCollisionAlogorithm(const ::std::vector<Box>& oldBoxList,
                                      ::std::vector<BoxInfo>& nextBox) const;
	
    void guyStep(const ::std::vector<Guy>& oldGuyList, unsigned int time, 
                 const ::std::vector<InputList>& playerInput, 
                 ::std::map<FrameID, MutableObjectList>& newDepartures,
                 ::std::vector<BoxInfo>& nextBox,
                 FrameID& currentPlayerFrame,
                 FrameID& nextPlayerFrame) const;

	bool wallAt(int x, int y) const;
    
    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    unsigned int timeLineLength;
    ::std::vector< ::std::vector<bool> > wallmap;
	int gravity;
	int wallSize;
};
}
#endif //HG_PHYSICS_ENGINE_H
