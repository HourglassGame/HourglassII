#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "InputList.h"
#include "Guy.h"
#include "Box.h"
#include "SimpleFrameID.h"
#include "TimeDirection.h"
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
                                        SimpleFrameID time,
                                        const ::std::vector<InputList>& playerInput,
                                        SimpleFrameID& currentPlayerFrame,
                                        TimeDirection& currentPlayerDirection,
                                        SimpleFrameID& nextPlayerFrame) const;

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

    void guyStep(const ::std::vector<Guy>& oldGuyList, 
                 SimpleFrameID time,
                 const ::std::vector<InputList>& playerInput,
                 ::std::map<SimpleFrameID, MutableObjectList>& newDepartures,
                 ::std::vector<BoxInfo>& nextBox,
                 SimpleFrameID& currentPlayerFrame,
                 SimpleFrameID& nextPlayerFrame,
                 TimeDirection& currentPlayerDirection) const;

	bool wallAt(int x, int y) const;
	bool wallAt(int x, int y, int w, int h) const;

    bool intersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const;

    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    unsigned int timeLineLength;
    ::std::vector< ::std::vector<bool> > wallmap;
	int gravity;
	int wallSize;
};
}
#endif //HG_PHYSICS_ENGINE_H
