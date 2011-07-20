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
#if 0
//RAII class for lua_State.
struct LuaState {
    LuaState() {
        ptr = lua_newstate();
    }
    ~LuaStae() {}
    lua_State* ptr;    
};
#endif
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

struct PositionAndVelocity2D {
    PositionAndVelocity2D(int x, int y, int xSpeed, int ySpeed) :
    xPositionAndVelocity(x, xSpeed),
    yPositionAndVelocity(y, ySpeed)
    {
    }
    PositionAndVelocity xPositionAndVelocity;
    PositionAndVelocity yPositionAndVelocity;
    
    int getX() const { return xPositionAndVelocity.position; }
    int getY() const { return yPositionAndVelocity.position; }
    int getXspeed() const{ return xPositionAndVelocity.velocity; }
    int getYspeed() const{ return yPositionAndVelocity.velocity; }
    
};
class NewOldTriggerSystem;
class NewOldTriggerFrameState {
public:
    PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            boost::transformed_range<GetBase<TriggerDataConstPtr>, std::vector<TriggerDataConstPtr> const > const& triggerArrivals);
   
    template<typename ObjectT>
    bool shouldPort(
        int responsiblePortalIndex,
        ObjectT const& potentialPorter)
    {
        return true;
    }
    template<typename ObjectT>
    bool shouldPickup(
        int responsiblePickupIndex,
        ObjectT const& potentialPickuper)
    {
        return true;
    }
    template<typename ObjectT>
    bool shouldDie(
        int responsibleKillerIndex,
        ObjectT const& potentialDier)
    {
        return true;
    }
    
    std::pair<
        std::map<Frame*, std::vector<TriggerData> >,
        std::vector<RectangleGlitz>
    > 
        getTriggerDeparturesAndGlitz(
            std::map<Frame*, ObjectList<Normal> > const& departures,
            Frame* currentFrame);
private:
    friend class NewOldTriggerSystem;
    NewOldTriggerFrameState(NewOldTriggerSystem const& triggerSys);
    
    std::vector<PositionAndVelocity2D> buttonStore;
    std::vector<std::vector<int> > triggers;
    std::vector<RectangleGlitz> glitzStore;
    NewOldTriggerSystem const& triggerSystem;
};

//The stuff needed to create a PortalArea, given trigger arrivals
struct ProtoPortal {
    PortalArea calculatePortalArea(std::vector<Collision> const&) const;
    ProtoPortal(
        Attachment const& attachment,
        int width,
        int height,
        TimeDirection timeDirection,
        int destinationIndex,
        int xDestination,
        int yDestination,
        bool relativeTime,
        int timeDestination,
        int illegalDestination,
        bool fallable,
        bool winner) :
    attachment_(attachment),
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    destinationIndex_(destinationIndex),
    xDestination_(xDestination),
    yDestination_(yDestination),
    relativeTime_(relativeTime),
    timeDestination_(timeDestination),
    illegalDestination_(illegalDestination),
    fallable_(fallable),
    winner_(winner)
    {}
private:
    //attachment_ has a dual meaning.
    //If attachment_.platformIndex == std::numeric_limits<std::size_t>::max()
    //then attachment_.xOffset and atachment_.yOffset give the absolute position of the portal.
    //Otherwise they give the position relative to the platform with index == attachment_.platformIndex
    Attachment attachment_;    
    int width_;
    int height_;
    TimeDirection timeDirection_;
    int destinationIndex_;
    int xDestination_;
    int yDestination_;
    bool relativeTime_;
    int timeDestination_;
    int illegalDestination_;
    bool fallable_;
    bool winner_;
};
//The stuff needed to create a CollisionArea, given trigger arrivals
//Also used to create the trigger departure relevant to the platform.
struct ProtoPlatform {
    ProtoPlatform(
        int width,
        int height,
        TimeDirection timeDirection,
        int lastStateTriggerID,
        int buttonTriggerID,
        PlatformDestinationPair const& destinations) :
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    lastStateTriggerID_(lastStateTriggerID),
    buttonTriggerID_(buttonTriggerID),
    destinations_(destinations)
    {
    }

    //uses trigger arrivals to generate Collision
    Collision calculateCollision(std::vector<std::vector<int> > const&) const;
    int width_;
    int height_;
    TimeDirection timeDirection_;
    //index of the trigger that holds [last-x-position, last-y-position, x-speed, y-speed]
    int lastStateTriggerID_;
    //index of the trigger that holds [on/off]
    //this is shared with the button controlling the platform.
    //The button writes to the trigger, the platform reads from the trigger
    int buttonTriggerID_;
    PlatformDestinationPair destinations_;
};

//Used to create the rectangle glitz for the button, as well as the trigger departure for the button.
struct ProtoButton {
    ProtoButton(
        Attachment const& attachment,
        int width,
        int height,
        TimeDirection timeDirection,
        int triggerID) :
    attachment_(attachment),
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    triggerID_(triggerID)
    {}


    PositionAndVelocity2D calculatePositionAndVelocity2D(std::vector<Collision> const& collisions) const;
    //See comment in ProtoPortal that explains the meaning of attachment_.
    Attachment attachment_;
    int width_;
    int height_;
    TimeDirection timeDirection_;
    //index of the trigger to which this button writes [on/off]
    int triggerID_;
};


//Old style construction interface, with new style back-end interface
class NewOldTriggerSystem {
public:
    NewOldTriggerFrameState getFrameState() const {
        return NewOldTriggerFrameState(*this);
    }
    
    NewOldTriggerSystem(
        std::vector<ProtoPortal> const& protoPortals,
        std::vector<ProtoPlatform> const& protoPlatforms,
        std::vector<ProtoButton> const& protoButtons,
        std::vector<std::pair<int, std::vector<int> > > const& ntriggerOffsetsAndDefaults) :
            protoPortals_(protoPortals),
            protoPlatforms_(protoPlatforms),
            protoButtons_(protoButtons),
            triggerOffsetsAndDefaults(ntriggerOffsetsAndDefaults)
        {
        }

private:
    
    //These store the information about portals/platforms that do not ever change.
    //This includes size, timedirection, winner, fallable etc..
    //This is just an example. These do not need to not ever change for the system to work.
    //However, it shows how constant stuff does not need to be sent through time.
    //In this case platforms just need to send their positions and velocities through time
    //and portals don't need to send anything, as they are just attached to platforms.
    std::vector<ProtoPortal> protoPortals_;
    std::vector<ProtoPlatform> protoPlatforms_;
    std::vector<ProtoButton> protoButtons_;

    //if we have Trigger trigger, and
    //int id = trigger.getIndex(); then
    //triggerOffsetsAndDefaults[id].first is the frame offset to which trigger departs and
    //triggerOffsetsAndDefaults[id].second is the default value to use of trigger in frames
    //where it does not arrive.
    std::vector<std::pair<int, std::vector<int> > > triggerOffsetsAndDefaults;
    
    friend class NewOldTriggerFrameState;
};
}
#endif //HG_TRIGGER_SYSTEM_H
