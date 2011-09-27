#ifndef HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#define HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "SimpleLuaCpp.h"
#include "ThreadLocal.h"
#include "SingleAssignmentPtr.h"
#include "ObjectAndTime.h"
#include <string>
#include <vector>
#include <boost/tuple/tuple.hpp>
namespace hg {
class DirectLuaTriggerFrameState :
    public TriggerFrameStateImplementation
{
    public:
    DirectLuaTriggerFrameState(
        LuaState& sharedState,
        std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const& triggerOffsetsAndDefaults,
        std::size_t arrivalLocationsSize);
    
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const* currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals);
    
    virtual bool shouldArrive(Guy const& potentialArriver);
    virtual bool shouldArrive(Box const& potentialArriver);
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter,
        bool porterActionedPortal);
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter,
        bool porterActionedPortal);
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Guy const& objectToManipulate);
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Box const& objectToManipulate);
    
    virtual boost::tuple<
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
        mt::std::vector<RectangleGlitz>::type,
        mt::std::vector<ObjectAndTime<Box> >::type
    > 
    getDepartureInformation(
        mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame);
    virtual ~DirectLuaTriggerFrameState();
private:
    LuaState& L_;

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


//The behaviour of this class is somewhat strange and not particularly useful in general, so the use
//of this class should be limited to cases where there are no better options.
template<typename T>
class lazy_ptr
{
public:
    lazy_ptr() :
        ptr_(new T())
    {
    }
    //Notice that these do not actually copy their arguments.
    //The arguments are not even moved. This is deliberate.
    lazy_ptr(lazy_ptr const&) :
        ptr_(new T())
    {
    }
    lazy_ptr& operator=(lazy_ptr const&)
    {
        //nothing to do.
    }
    ~lazy_ptr()
    {
        delete ptr_;
    }
    T& operator*() const
    {
        return *ptr_;
    }
    T* operator->() const
    {
        return ptr_;
    }
    T* get() const
    {
        return ptr_;
    }
private:

    T* ptr_;
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
    virtual TriggerFrameState getFrameState() const;
    virtual TriggerSystemImplementation* clone() const
    {
        return new DirectLuaTriggerSystem(*this);
    }
private:
    lazy_ptr<ThreadLocal<SingleAssignmentPtr<LuaState> > > luaStates_;
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
