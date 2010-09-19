#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "InputList.h"
#include "Guy.h"
#include "Box.h"
#include "NewFrameID.h"
#include "TimeDirection.h"
#include "ObjectList.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include <vector>
#include <map>
namespace hg {
class ObjectList;
class TimeObjectListList;
class PhysicsEngine
{
public:
	PhysicsEngine(  ::std::vector< ::std::vector<bool> > newWallmap,
                    int newWallSize,
                    int newGravity,
                    AttachmentMap nAttachmentMap,
                    TriggerSystem nTriggerSystem);

    // executes frame and returns departures
	TimeObjectListList executeFrame(const ObjectList& arrivals,
                                        NewFrameID time,
                                        const ::std::vector<InputList>& playerInput,
                                        NewFrameID& currentPlayerFrame,
                                        TimeDirection& currentPlayerDirection,
                                        NewFrameID& nextPlayerFrame) const;

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

    void platformStep(const ::std::vector<Platform>& oldPlatformList,
                      const std::vector<PlatformDestination>& platformDestinations,
                      ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                      const NewFrameID& time) const;

    void buttonChecks(const ::std::vector<Box>& oldBoxList,
                    const ::std::vector<Guy>& oldGuyList,
                    const ::std::vector<Button>& oldButtonList,
                    ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                    NewFrameID time) const;

	void crappyBoxCollisionAlogorithm(const ::std::vector<Box>& oldBoxList,
                                      ::std::vector<BoxInfo>& nextBox,
                                      const ::std::map<NewFrameID, MutableObjectList>& newDepartures) const;

    void guyStep(const ::std::vector<Guy>& oldGuyList,
                 NewFrameID time,
                 const ::std::vector<InputList>& playerInput,
                 ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                 ::std::vector<BoxInfo>& nextBox,
                 NewFrameID& currentPlayerFrame,
                 NewFrameID& nextPlayerFrame,
                 TimeDirection& currentPlayerDirection) const;

	bool wallAt(int x, int y) const;
	bool wallAt(int x, int y, int w, int h) const;

    bool intersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const;

    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    ::std::vector< ::std::vector<bool> > wallmap;
	int gravity;
	int wallSize;
	AttachmentMap attachmentMap;
	TriggerSystem triggerSystem;
};
}
#endif //HG_PHYSICS_ENGINE_H
