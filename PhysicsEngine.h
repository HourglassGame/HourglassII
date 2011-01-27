#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "InputList.h"
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "NewFrameID.h"
#include "TimeDirection.h"
#include "ObjectList.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include "RemoteDepartureEdit.cpp"
#include <boost/multi_array.hpp>
#include <vector>
#include <map>
namespace hg {
class ObjectList;
class TimeObjectListList;
class ConcurrentTimeMap;
class ConcurrentTimeSet;
class PhysicsEngine
{
public:
	PhysicsEngine(  const ::boost::multi_array<bool, 2>& nwallmap,
                    int newWallSize,
                    int newGravity,
                    const AttachmentMap& nAttachmentMap,
                    const TriggerSystem& nTriggerSystem);

    // executes frame and returns departures
	TimeObjectListList executeFrame(const ObjectList& arrivals,
                                    NewFrameID time,
                                    const ::std::vector<InputList>& playerInput,
                                    ConcurrentTimeMap& currentPlayerFramesAndDirections,
                                    ConcurrentTimeSet& nextPlayerFrames,
                                    ConcurrentTimeSet& winFrames) const;

private:
    struct BoxInfo {
        BoxInfo(Box nbox,
                NewFrameID nTime,
                bool nsupported) :
        info(nbox),
        time(nTime),
        supported(nsupported)
        {}
        Box info;
        NewFrameID time;
        bool supported;
    };

    struct GuyInfo {
        GuyInfo(Guy nGuy,
                NewFrameID nTime) :
        info(nGuy),
        time(nTime)
        {}
        Guy info;
        NewFrameID time;
    };

    void platformStep(const ::std::vector<Platform>& oldPlatformList,
                      ::std::vector<Platform>& nextPlatform,
                      const std::vector<PlatformDestination>& platformDestinations,
                      const NewFrameID& time) const;

    void buttonChecks(  const ::std::vector<Platform>& oldPlatformList,
                        const ::std::vector<Box>& oldBoxList,
                        const ::std::vector<Guy>& oldGuyList,
                        const ::std::vector<Button>& oldButtonList,
                        ::std::vector<bool>& nextButtonState,
                        NewFrameID time) const;

    template <class Type, class TypeInfo> void BuildDepartureForComplexEntities(
                                    const ::std::vector<TypeInfo>& next,
                                    const ::std::vector<RemoteDepartureEdit<Type> >& thief,
                                    const ::std::vector<RemoteDepartureEdit<Type> >& extra,
                                    ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                                    const NewFrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const;

    template <class Type> void BuildDepartureForReallySimpleThing(
                                    const ::std::vector<Type>& next,
                                    ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                                    const NewFrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const;

    void buildDepartures(const ::std::vector<BoxInfo>& nextBox,
                        const ::std::vector<Platform>& nextPlatform,
                        const ::std::vector<Portal>& nextPortal,
                        const ::std::vector<Button>& nextButton,
                        const ::std::vector<GuyInfo>& nextGuy,
                        const ::std::vector<RemoteDepartureEdit<Box> >& boxThief,
                        const ::std::vector<RemoteDepartureEdit<Box> >& boxExtra,
                        const ::std::vector<RemoteDepartureEdit<Guy> >& guyExtra,
                        ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                        const NewFrameID time,
                        std::vector<PauseInitiatorID>& pauseTimes
                        ) const;

    void buttonPositionUpdate(
        const ::std::vector<Platform>& nextPlatform,
        const ::std::vector<bool>& nextButtonState,
        const ::std::vector<Button>& oldButtonList,
        ::std::vector<Button>& nextButton,
        NewFrameID time
    ) const;

    void portalPositionUpdate(
        const ::std::vector<Platform>& nextPlatform,
        const ::std::vector<Portal>& oldPortalList,
        ::std::vector<Portal>& nextPortal,
        NewFrameID time
    ) const;

	void crappyBoxCollisionAlogorithm(  const ::std::vector<Box>& oldBoxList,
                                        ::std::vector<BoxInfo>& nextBox,
                                        const ::std::vector<Platform>& nextPlatform,
                                        const NewFrameID time
                                        ) const;

    void guyStep(   const ::std::vector<Guy>& oldGuyList,
                    NewFrameID time,
                    const ::std::vector<InputList>& playerInput,
                    ::std::vector<GuyInfo>& nextGuy,
                    ::std::vector<BoxInfo>& nextBox,
                    const ::std::vector<Platform>& nextPlatform,
                    const ::std::vector<Portal>& nextPortal,
                    ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                    ConcurrentTimeMap& currentPlayerFramesAndDirections,
                    ConcurrentTimeSet& nextPlayerFrames,
                    std::vector<PauseInitiatorID>& pauseTimes) const;

	bool wallAt(int x, int y) const;
	bool wallAt(int x, int y, int w, int h) const;

    bool PointInRectangle(int px, int py, int x, int y, int w, int h, bool inclusive) const;
    bool IntersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const;

    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    ::boost::multi_array<bool, 2> wallmap;
	int gravity;
	int wallSize;
	AttachmentMap attachmentMap;
	TriggerSystem triggerSystem;
};
}
#endif //HG_PHYSICS_ENGINE_H
