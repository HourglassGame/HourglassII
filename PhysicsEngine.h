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
#include <boost/fusion/include/vector.hpp>
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
    PhysicsEngine(
        const boost::multi_array<bool, 2>& nwallmap,
        int newWallSize,
        int newGravity,
        const AttachmentMap& nAttachmentMap,
        const TriggerSystem& nTriggerSystem
       );

    struct PhysicsReturnT
    {
    	PhysicsReturnT(std::map<Frame*, ObjectList> Ndepartures,
    			bool NcurrentPlayerFrame,
    			bool NnextPlayerFrame,
    			bool NcurrentWinFrame) :
    				departures(Ndepartures),
    				currentPlayerFrame(NcurrentPlayerFrame),
    				nextPlayerFrame(NnextPlayerFrame),
    				currentWinFrame(NcurrentWinFrame)
    	{}
    	std::map<Frame*, ObjectList> departures;
    	bool currentPlayerFrame;
    	bool nextPlayerFrame;
    	bool currentWinFrame;
    };

    // executes frame and returns departures
    PhysicsEngine::PhysicsReturnT executeFrame(
        const ObjectPtrList& arrivals,
        Frame* time,
        const std::vector<InputList>& playerInput) const;

private:
    typedef std::map<Frame*, ObjectList> NewDeparturesT;

    template<typename Object>
    struct ObjectAndTime
    {
        ObjectAndTime(const Object& nobject,
                Frame* nTime) :
                object(nobject),
                time(nTime)
        {}
        Object object;
        Frame* time;
    };
/*
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
    };*/

    void platformStep(
        const std::vector<const Platform*>& oldPlatformList,
        std::vector<Platform>& nextPlatform,
        const std::vector<PlatformDestination>& pd) const;

    void buttonChecks(
        const std::vector<const Platform*>& oldPlatformList,
        const std::vector<const Box*>& oldBoxList,
        const std::vector<const Guy*>& oldGuyList,
        const std::vector<const Button*>& oldButtonList,
        std::vector<char>& nextButton) const;

    template <class Type> void buildDeparturesForComplexEntities(
        const std::vector<ObjectAndTime<Type> >& next,
        const std::vector<const RemoteDepartureEdit<Type>* >& thieves,
        const std::vector<const RemoteDepartureEdit<Type>* >& extras,
        NewDeparturesT& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes) const;

    template <class Type> void buildDeparturesForReallySimpleThings(
        const std::vector<Type>& next,
        NewDeparturesT& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes) const;

    void buildDepartures(
        const std::vector<ObjectAndTime<Box> >& nextBox,
        const std::vector<Platform>& nextPlatform,
        const std::vector<Portal>& nextPortal,
        const std::vector<Button>& nextButton,
        const std::vector<ObjectAndTime<Guy> >& nextGuy,
        const std::vector<const RemoteDepartureEdit<Box>* >& boxThieves,
        const std::vector<const RemoteDepartureEdit<Box>* >& extraBoxes,
        const std::vector<const RemoteDepartureEdit<Guy>* >& extraGuys,
        NewDeparturesT& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes) const;

    void buttonPositionUpdate(
        const std::vector<Platform>& nextPlatform,
        const std::vector<char>& nextButtonState,
        const std::vector<const Button*>& oldButtonList,
        std::vector<Button>& nextButton) const;

    void portalPositionUpdate(
        const std::vector<Platform>& nextPlatform,
        const std::vector<const Portal*>& oldPortalList,
        std::vector<Portal>& nextPortal) const;

    bool explodeBoxes(
		std::vector<int>& pos,
		std::vector<int>& size,
		std::vector<std::vector<int> >& links,
		std::vector<char>& toBeSquished,
		std::vector<int>& bound,
		int index,
		int boundSoFar,
		int sign) const;

    bool explodeBoxesUpwards(
		std::vector<int>& x,
		std::vector<int>& xTemp,
		std::vector<int>& y,
		std::vector<int>& size,
		std::vector<std::vector<int> >& links,
		bool firstTime,
    	std::vector<char>& toBeSquished,
    	std::vector<int>& bound,
    	int index,
    	int boundSoFar) const;


    void recursiveBoxCollision(
		std::vector<int>& majorAxis,
		std::vector<int>& minorAxis,
		std::vector<int>& size,
		std::vector<char>& squished,
		std::vector<int>& boxesSoFar,
		unsigned int index) const;

    void boxCollisionAlogorithm(
        const std::vector<const Box*>& oldBoxList,
        std::vector<ObjectAndTime<Box> >& nextBox,
        std::vector<Platform>& nextPlatform,
        Frame* time) const;

    void guyStep(
        const std::vector<const Guy*>& oldGuyList,
        Frame* time,
        const std::vector<InputList>& playerInput,
        std::vector<ObjectAndTime<Guy> >& nextGuy,
        std::vector<ObjectAndTime<Box> >& nextBox,
        const std::vector<Platform>& nextPlatform,
        const std::vector<Portal>& nextPortal,
        NewDeparturesT& newDepartures,
        bool& currentPlayerFrame,
        bool& nextPlayerFrame,
        bool& winFrame,
        std::vector<PauseInitiatorID>& pauseTimes) const;

    bool wallAt(int x, int y) const;
    bool wallAt(int x, int y, int w, int h, bool inclusive) const;

    //map info (keeping it here allows for an optimised representation;
    //          also, the fact that the physics engine uses a world should be irrelevant to the time-engine)
    boost::multi_array<bool, 2> wallmap;
    int gravity;
    int wallSize;
    AttachmentMap attachmentMap;
    TriggerSystem triggerSystem;
};
}
#endif //HG_PHYSICS_ENGINE_H
