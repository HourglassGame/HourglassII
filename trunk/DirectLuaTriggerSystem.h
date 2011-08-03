#ifndef HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#define HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include <string>
#include <vector>
#include "SimpleLuaCpp.h"
namespace hg {
class DirectLuaTriggerFrameState :
    public TriggerFrameStateImplementation
{
    public:
    DirectLuaTriggerFrameState(
        std::vector<char> const& compiledLuaChunk,
        std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const& triggerOffsetsAndDefaults,
        std::size_t arrivalLocationsSize);
    
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals);
                
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter);
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter);
        
    virtual bool shouldPickup(
        int responsiblePickupIndex,
        Guy const& potentialPickuper);
    virtual bool shouldPickup(
        int responsiblePickupIndex,
        Box const& potentialPickuper);
        
    virtual bool shouldDie(
        int responsibleKillerIndex,
        Guy const& potentialDier);
    virtual bool shouldDie(
        int responsibleKillerIndex,
        Box const& potentialDier);
    
    virtual std::pair<
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
        mt::std::vector<RectangleGlitz>::type
    > 
    getTriggerDeparturesAndGlitz(
        mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame);

private:
    LuaState L_;

    std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const& triggerOffsetsAndDefaults_;

    //Gives the size that arrivalLocations must always be,
    //for script-validation purposes.
    //If a portal specifies an destinationIndex that is
    //larger than this
    std::size_t arrivalLocationsSize_;

    DirectLuaTriggerFrameState(DirectLuaTriggerFrameState& other);
    DirectLuaTriggerFrameState& operator=(DirectLuaTriggerFrameState& other);
};
class DirectLuaTriggerSystem :
    public TriggerSystemImplementation
{
public:
    DirectLuaTriggerSystem(
        std::vector<char> const& triggerSystemLuaChunk,
        std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
        > const& triggerOffsetsAndDefaults,
        std::size_t arrivalLocationsSize);
    virtual TriggerFrameState getFrameState() const
    {
        //Unfortunately (due to the lack of perfect forwarding in C++03)
        //this cannot be made into a function.
        //This should be the equivalent to:
        //  return new DirectLuaTriggerFrameState(compiledLuaChunk);
        //except using a custom allocation function.
        void* p(0);
        try {
            p = multi_thread_operator_new(sizeof(DirectLuaTriggerFrameState));
            return TriggerFrameState(
                new (p) DirectLuaTriggerFrameState(
                    compiledLuaChunk_, triggerOffsetsAndDefaults_, arrivalLocationsSize_));
        }
        catch (...) {
            multi_thread_operator_delete(p);
            throw;
        }
    }
    virtual TriggerSystemImplementation* clone() const
    {
        return new DirectLuaTriggerSystem(*this);
    }
private:
    std::vector<char> compiledLuaChunk_;
    std::vector<
        std::pair<
            int,
            std::vector<int>
        >
    > triggerOffsetsAndDefaults_;
    std::size_t arrivalLocationsSize_;
};
}
#endif //HG_DIRECT_LUA_TRIGGER_SYSTEM_H
