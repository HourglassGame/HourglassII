#ifndef HG_BASIC_CONFIURED_TRIGGER_SYSTEM_H
#define HG_BASIC_CONFIURED_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "ProtoStuff.h"
#include "ObjectPtrList.h"
#include "RectangleGlitz.h"
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "Frame.h"
#include "multi_thread_allocator.h"
namespace hg {
class BasicConfiguredTriggerSystem;
class BasicConfiguredTriggerFrameState :
    public TriggerFrameStateImplementation
{
    public:
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const* /*currentFrame*/,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals);
                
    virtual bool shouldPort(
        int /*responsiblePortalIndex*/,
        Guy const& /*potentialPorter*/) { return true; }
    virtual bool shouldPort(
        int /*responsiblePortalIndex*/,
        Box const& /*potentialPorter*/) { return true; }
        
    virtual bool shouldPickup(
        int /*responsiblePickupIndex*/,
        Guy const& /*potentialPickuper*/) { return true; }
    virtual bool shouldPickup(
        int /*responsiblePickupIndex*/,
        Box const& /*potentialPickuper*/) { return true; }
        
    virtual bool shouldDie(
        int /*responsibleKillerIndex*/,
        Guy const& /*potentialDier*/) { return true; }
    virtual bool shouldDie(
        int /*responsibleKillerIndex*/,
        Box const& /*potentialDier*/) { return true; }
    
    virtual std::pair<
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
        mt::std::vector<RectangleGlitz>::type
    > 
    getTriggerDeparturesAndGlitz(
        mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame);
    
    friend class BasicConfiguredTriggerSystem;
    BasicConfiguredTriggerFrameState(BasicConfiguredTriggerSystem const& triggerSys);
    
    mt::std::vector<PositionAndVelocity2D>::type buttonStore;
    mt::std::vector<mt::std::vector<int>::type >::type triggers;
    mt::std::vector<RectangleGlitz>::type glitzStore;
    BasicConfiguredTriggerSystem const& triggerSystem;
    
};

class BasicConfiguredTriggerSystem :
    public TriggerSystemImplementation
{
    public:
    virtual TriggerFrameState getFrameState() const {
        //Unfortunately (due to the lack of perfect forwarding in C++03)
        //this cannot be made into a function.
        //This should be the equivalent to:
        //  return new BasicConfiguredTriggerFrameState(*this);
        //except using a custom allocation function.
        void* p(multi_thread_operator_new(sizeof(BasicConfiguredTriggerFrameState)));
        try {
            return TriggerFrameState(new (p) BasicConfiguredTriggerFrameState(*this));
        }
        catch (...) {
            multi_thread_operator_delete(p);
            throw;
        }
    }
    
    virtual BasicConfiguredTriggerSystem* clone() const {
        return new BasicConfiguredTriggerSystem(*this);
    }
    
    BasicConfiguredTriggerSystem(
        std::vector<ProtoPortal> const& protoPortals,
        std::vector<ProtoPlatform> const& protoPlatforms,
        std::vector<ProtoButton> const& protoButtons,
        std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const& ntriggerOffsetsAndDefaults) :
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
    
    friend class BasicConfiguredTriggerFrameState;
};
}
#endif //HG_BASIC_CONFIURED_TRIGGER_SYSTEM_H
