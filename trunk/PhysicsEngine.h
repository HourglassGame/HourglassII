#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "InputList.h"
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "FrameID.h"
#include "TimeDirection.h"
#include "ObjectList.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include "RemoteDepartureEdit.h"
#include <boost/multi_array.hpp>
#include <vector>
#include <map>
namespace hg {
class Frame;
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
	std::map<Frame*, ObjectList> executeFrame(const ObjectPtrList& arrivals,
                                    Frame* time,
                                    const ::std::vector<InputList>& playerInput,
                                    ConcurrentTimeMap& currentPlayerFramesAndDirections,
                                    ConcurrentTimeSet& nextPlayerFrames,
                                    ConcurrentTimeSet& winFrames) const;

private:
    typedef std::map<Frame*, ObjectList> NewDeparturesT;
    
    struct BoxInfo {
        BoxInfo(Box nbox,
                Frame* nTime) :
        info(nbox),
        time(nTime)
        {}
        Box info;
        Frame* time;
    };

    struct GuyInfo {
        GuyInfo(Guy nGuy,
                Frame* nTime) :
        info(nGuy),
        time(nTime)
        {}
        Guy info;
        Frame* time;
    };

    void platformStep(const ::std::vector<const Platform*>& oldPlatformList,
                      std::vector<Platform>& nextPlatform,
                      const std::vector<PlatformDestination>& pd,
                      Frame* time) const;

    void buttonChecks(  const ::std::vector<const Platform*>& oldPlatformList,
                        const ::std::vector<const Box*>& oldBoxList,
                        const ::std::vector<const Guy*>& oldGuyList,
                        const ::std::vector<const Button*>& oldButtonList,
                        std::vector<char>& nextButton,
                        Frame* time) const;

    template <class Type, class TypeInfo> void BuildDepartureForComplexEntities(
                                    const ::std::vector<TypeInfo>& next,
                                    const ::std::vector<const RemoteDepartureEdit<Type>* >& thief,
                                    const ::std::vector<const RemoteDepartureEdit<Type>* >& extra,
                                    NewDeparturesT& newDepartures,
                                    Frame* time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const;

    template <class Type> void BuildDepartureForReallySimpleThing(
                                    const ::std::vector<Type>& next,
                                    NewDeparturesT& newDepartures,
                                    Frame* time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const;

    void buildDepartures(const ::std::vector<BoxInfo>& nextBox,
                        const ::std::vector<Platform>& nextPlatform,
                        const ::std::vector<Portal>& nextPortal,
                        const ::std::vector<Button>& nextButton,
                        const ::std::vector<GuyInfo>& nextGuy,
                        const ::std::vector<const RemoteDepartureEdit<Box>* >& boxThief,
                        const ::std::vector<const RemoteDepartureEdit<Box>* >& boxExtra,
                        const ::std::vector<const RemoteDepartureEdit<Guy>* >& guyExtra,
                        NewDeparturesT& newDepartures,
                        Frame* time,
                        std::vector<PauseInitiatorID>& pauseTimes
                        ) const;

    void buttonPositionUpdate(
        const ::std::vector<Platform>& nextPlatform,
        const ::std::vector<char>& nextButtonState,
        const ::std::vector<const Button*>& oldButtonList,
        ::std::vector<Button>& nextButton,
        Frame* time
    ) const;

    void portalPositionUpdate(
        const std::vector<Platform>& nextPlatform,
        const ::std::vector<const Portal*>& oldPortalList,
        std::vector<Portal>& nextPortal,
        Frame* time
    ) const;

	void crappyBoxCollisionAlogorithm(  const ::std::vector<const Box*>& oldBoxList,
                                        ::std::vector<BoxInfo>& nextBox,
                                        std::vector<Platform>& nextPlatform,
                                        Frame* time
                                        ) const;

    void guyStep(   const ::std::vector<const Guy*>& oldGuyList,
                    Frame* time,
                    const ::std::vector<InputList>& playerInput,
                    std::vector<GuyInfo>& nextGuy,
                    ::std::vector<BoxInfo>& nextBox,
                    const ::std::vector<Platform>& nextPlatform,
                    const ::std::vector<Portal>& nextPortal,
                    NewDeparturesT& newDepartures,
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