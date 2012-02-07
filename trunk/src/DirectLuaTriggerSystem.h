#ifndef HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#define HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "SimpleLuaCpp.h"
#include "ThreadLocal.h"
#include "ObjectAndTime.h"
#include <string>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/move/move.hpp>
namespace hg {
class OperationInterrupter;
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
        std::size_t arrivalLocationsSize/*,
        OperationInterrupter& interrupter*/);
    
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const* currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr>::type const> const& triggerArrivals/*
            ,OperationInterrupter& interrupter*/);
    
    virtual bool shouldArrive(Guy const& potentialArriver/*,OperationInterrupter& interrupter*/);
    virtual bool shouldArrive(Box const& potentialArriver/*,OperationInterrupter& interrupter*/);
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter,
        bool porterActionedPortal/*,
        OperationInterrupter& interrupter*/);
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter,
        bool porterActionedPortal/*,
        OperationInterrupter& interrupter*/);
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Guy const& objectToManipulate/*,
        OperationInterrupter& interrupter*/);
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Box const& objectToManipulate/*,
        OperationInterrupter& interrupter*/);
    
    virtual DepartureInformation getDepartureInformation(
        mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame/*,
        OperationInterrupter& interrupter*/);
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
    //larger than this, it is an error in the script and will be detected as such.
    std::size_t arrivalLocationsSize_;

    DirectLuaTriggerFrameState(DirectLuaTriggerFrameState& o);
    DirectLuaTriggerFrameState& operator=(DirectLuaTriggerFrameState& o);
};


//The behaviour of this class is somewhat strange and not particularly useful
//in general, so the use of this class should be limited to cases where there
//are no better options.
//!!WARNING - The const members of this class cannot be regarded as
//safe when concurrently called on the same object!! Mutable is in use.
template<typename T>
class lazy_ptr
{
public:
    lazy_ptr() :
        ptr_(0)
    {
    }
    //Notice that these do not actually copy their arguments.
    //The arguments are not even moved. This is deliberate.
    lazy_ptr(lazy_ptr const&) :
        ptr_(0)
    {
    }
    lazy_ptr& operator=(BOOST_COPY_ASSIGN_REF(lazy_ptr))
    {
        //nothing to do.
    	return *this;
    }
    lazy_ptr(BOOST_RV_REF(lazy_ptr) o) :
    	ptr_(0)
    {
    	boost::swap(ptr_, o.ptr_);
    }
    lazy_ptr& operator=(BOOST_RV_REF(lazy_ptr) o)
    {
    	boost::swap(ptr_, o.ptr_);
    	return *this;
    }
    ~lazy_ptr()
    {
        delete ptr_;
    }
    T& operator*() const
    {
    	if (!ptr_) ptr_ = new T();
        return *ptr_;
    }
    T* operator->() const
    {
    	if (!ptr_) ptr_ = new T();
        return ptr_;
    }
    T* get() const
    {
    	if (!ptr_) ptr_ = new T();
        return ptr_;
    }
private:
    mutable T* ptr_;
    BOOST_COPYABLE_AND_MOVABLE(lazy_ptr)
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
    virtual TriggerFrameState getFrameState(/*OperationInterrupter& interrupter*/) const;
    virtual TriggerSystemImplementation* clone() const
    {
        return new DirectLuaTriggerSystem(*this);
    }
private:
    //lazy_ptr because TriggerSystemImplementations must
    //be cloneable, but there is no way to copy
    //a LuaState (or by extension a ThreadLocal<LuaState>).
    //lazy_ptr side-steps this problem by making a fresh
    //instance of the ThreadLocal<LuaState> at every step.
    //luaStates_ a cache, so ignoring its contents does not cause
    //any problems.
    lazy_ptr<ThreadLocal<LuaState> > luaStates_;
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
