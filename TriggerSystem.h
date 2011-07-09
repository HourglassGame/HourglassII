#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H

#include "Button.h"
#include "PlatformDestination.h"
#include "AttachmentMap.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "InputList.h"
#include "Frame_fwd.h"
#include "Box.h"
#include "Collision.h"
#include "PortalArea.h"
#include "PickupArea.h"
#include "KillerArea.h"
#include "ArrivalLocation.h"
#include "RectangleGlitz.h"
#include <vector>
#include <map>
namespace hg
{

struct PhysicsAffectingStuff {
    std::vector<Box> additionalBoxes;
    std::vector<Collision> collisions;
    std::vector<PortalArea> portals;
    std::vector<PickupArea> pickups;
    std::vector<KillerArea> killers;
    //guaranteed to always contain elements at each index that could possibly be indexed
    //ie- arrivalLocations will always be the same length for a particular NewTriggerSystem
    std::vector<ArrivalLocation> arrivalLocations;
};
class NewTriggerSystem;
//Does the actual work, 1 created per frame
//Has a life cycle:
//[Created ->
//  calculate physics affecting stuff from trigger arrivals ->
//  Have callins called ->
//  calculate glitz and trigger departures]
class TriggerFrameState {
public:
    //physics affecting stuff
    template<typename TriggerDataConstPtrRange>
    PhysicsAffectingStuff calculatePhysicsAffectingStuff(TriggerDataConstPtrRange const& triggerArrivals);
    
    //Callins --
    //ObjectT can be Box or Guy atm
    template<typename ObjectT>
    bool shouldPort(int responsiblePortalIndex, ObjectT const& potentialPorter);
    template<typename ObjectT>
    bool shouldPickup(int responsiblePickupIndex, ObjectT const& potentialPickuper);
    template<typename ObjectT>
    bool shouldDie(int responsibleKillerIndex, ObjectT const& potentialDier);
    
    //Calls could be interleaved/these two could be made one function which returns a pair.
    std::vector<RectangleGlitz> 
        getGlitz(std::map<Frame*, ObjectList<Normal> > const& departures) const;

    std::map<Frame*, std::vector<TriggerData> >
        getTriggerDepartures(std::map<Frame*, ObjectList<Normal> > const& departures) const;

private:
    //Allows the implementation of NewTriggerSystem to actually construct these TriggerFrameState objects.
    friend class NewTriggerSystem;
    TriggerFrameState();
    TriggerFrameState(TriggerFrameState const&);
    TriggerFrameState& operator=(TriggerFrameState const&);
    /**private data**/
};
//Factory for TriggerFrameStates
class NewTriggerSystem {
    public:
    TriggerFrameState getFrameState() const;
};

///*** IGNORE THE FOLLOWING CODE --- IT IS ONLY HERE FOR SUPPORT OF THE OLD SYSTEM ***

//definition: Statically identifiable object -- object which never arrives more than once in a particular frame
//                                              [note: which therefore can be unabiguouly identified by its index]
//A pluggable component for the physics engine which allows arbitrary code to be executed to calculate the departures
//of staticly identifiable objects based on arrivals to a frame. Also has capabilites for adding extra "TriggerObjects",
//which allow it to communicate with itself through time.
class TriggerSystem {
public:
//The current version is a crappified and cleaned-up version of the original, which also include all the static object 
//physics from the physics engine.
//The old model was "TriggerSystem is a system for combining button states into platform destinations"
//The old code did not actually manage that, but I can only assume it was an implementation issue.
//New model is described above. The new model may be overly general, but it can be cut back as required.
//This particular incarnation requires an exact 1 to 1 to 1 relationship between buttons, platforms and destinations.
//That is: each button corresponds to a pair of destinations, which is selected between when doing platform physics
//This version can, however, handle the situation where objects arrivals have not yet arrived, and so 
//particular platforms/buttons/etc have not yet come into existence.
TriggerSystem(
    unsigned int buttonCount,
    unsigned int triggerCount,
    const std::vector<int> triggerOffset,
    const std::vector<PlatformDestinationPair>& destinations,
    const AttachmentMap& attachmentMap);


//Creates Departures for all arrivals of pauseLevel == 0 staticly identifiable objects in arrivals
//These objects must also be in the top-level universe (as otherwise they would not be statically identifiable)
//Therefore, if !(getInitiatorFrame(getUniverse(time))) then this returns a map which contains no statically identifiable object departures.
//Also creates TriggerObjects. These are a trigger ID combined with arbitrary data 
//(stored as an int for now, could be changed to be something else if necessecary), which the time engine simply makes depart.
//Could also return a special ObjectList<Normal> -ish thing which contains instructions for physics (such as, "create pause here" etc.)
//This would, however, complicate the interface, so I'd like to not do it unless there is a specific reason/level that
//needs the feature.
//Note: The time-engine is still responsible for adding the "pause-time departures" of objects with pauseLevel != 0.
ObjectList<Normal>  calculateStaticDepartures(
    const ObjectPtrList<Normal> & arrivals,
    const std::vector<InputList>& playerInput,
    const Frame* time) const;
private:
    unsigned int buttonCount_;
    unsigned int triggerCount_;
    std::vector<int> triggerOffset_;
    std::vector<PlatformDestinationPair> destinations_;
    AttachmentMap attachmentMap_;
};
}
#endif //HG_TRIGGER_SYSTEM_H
