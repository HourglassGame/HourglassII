#ifndef HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#define HG_DIRECT_LUA_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "SimpleLuaCpp.h"
#include "ThreadLocal.h"
#include "ObjectAndTime.h"
#include "LuaInterruption.h"
#include "LuaModule.h"
#include <string>
#include <vector>
#include <mutex>

#include <boost/optional.hpp>
#include <boost/polymorphic_cast.hpp>
#include <tuple>
namespace hg {
class OperationInterrupter;
class DirectLuaTriggerFrameState final :
    public TriggerFrameStateImplementation
{
    public:
    DirectLuaTriggerFrameState(
        LuaState &sharedState,
        std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const &triggerOffsetsAndDefaults,
        std::size_t arrivalLocationsSize,
        OperationInterrupter &interrupter);
    
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const *currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) override;
    
    virtual bool shouldArrive(Guy const &potentialArriver) override;
    virtual bool shouldArrive(Box const &potentialArriver) override;
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const &potentialPorter,
        bool porterActionedPortal) override;
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const &potentialPorter,
        bool porterActionedPortal) override;
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int> const &responsibleMutatorIndices,
        Guy const &objectToManipulate) override;
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int> const &responsibleMutatorIndices,
        Box const &objectToManipulate) override;
    
    virtual DepartureInformation getDepartureInformation(
        mt::std::map<Frame*, ObjectList<Normal>> const &departures,
        Frame *currentFrame) override;
    virtual ~DirectLuaTriggerFrameState() noexcept override;
private:
    OperationInterrupter &interrupter_;
    LuaState &L_;

    std::vector<
            std::pair<
                int,
                std::vector<int>
            >
        > const &triggerOffsetsAndDefaults_;

    //Gives the size that arrivalLocations must always be,
    //for script-validation purposes.
    //If a portal specifies an destinationIndex that is
    //larger than this, it is an error in the script and will be detected as such.
    std::size_t arrivalLocationsSize_;

    LuaInterruptionHandle interruptionHandle_;

    DirectLuaTriggerFrameState(DirectLuaTriggerFrameState &o) = delete;
    DirectLuaTriggerFrameState &operator=(DirectLuaTriggerFrameState &o) = delete;
    DirectLuaTriggerFrameState(DirectLuaTriggerFrameState &&o) = delete;
    DirectLuaTriggerFrameState &operator=(DirectLuaTriggerFrameState &&o) = delete;
};


//The behaviour of this class is somewhat strange and not particularly useful
//in general, so the use of this class should be limited to cases where there
//are no better options.
template<typename T>
class copy_as_new_ptr final
{
public:
    copy_as_new_ptr() :
        ptr(new T())
    {
    }
    //Notice that these do not actually copy their arguments.
    //The arguments are not even moved. This is deliberate.
    copy_as_new_ptr(copy_as_new_ptr const &) :
        ptr(new T())
    {
    }
    copy_as_new_ptr &operator=(copy_as_new_ptr o)
    {
        return *this = std::move(o);
    }

    //Move operations are not useful, because they cannot be made no-throw.
    copy_as_new_ptr(copy_as_new_ptr &&o) = delete;
    copy_as_new_ptr &operator=(copy_as_new_ptr &&o) = delete;
    ~copy_as_new_ptr() noexcept
    {
        delete ptr;
    }
    T &operator*() const
    {
        return get();
    }
    T *operator->() const
    {
        return get();
    }
    T *get() const
    {
        assert(ptr);
        return ptr;
    }
private:
    T *ptr;
};

class DirectLuaTriggerSystem final :
    public TriggerSystemImplementation
{

    auto comparison_tuple() const -> decltype(auto)
    {
        //Doesn't include luaStates_, since luaStates_ is just a cache.
        return std::tie(compiledMainChunk_, compiledExtraChunks_, triggerOffsetsAndDefaults_, arrivalLocationsSize_);
    }
public:
    DirectLuaTriggerSystem(
        std::vector<char> const &mainChunk,
        std::vector<LuaModule> const &extraChunks,
        std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
        > triggerOffsetsAndDefaults,
        std::size_t arrivalLocationsSize);
    virtual TriggerFrameState getFrameState(OperationInterrupter &interrupter) const override;
    virtual TriggerSystemImplementation *clone() const override
    {
        return new DirectLuaTriggerSystem(*this);
    }
    virtual bool operator==(TriggerSystemImplementation const &o) const override
    {
        DirectLuaTriggerSystem const &actual_other(*boost::polymorphic_downcast<DirectLuaTriggerSystem const*>(&o));
        return comparison_tuple() == actual_other.comparison_tuple();
    }
    virtual int order_ranking() const override
    {
        return 1000;
    }
private:
    //lazy_ptr because TriggerSystemImplementations must
    //be cloneable, but there is no way to copy
    //a LuaState (or by extension a ThreadLocal<LuaState>).
    //copy_as_new_ptr side-steps this problem by making copying create
    //a fresh instance of the ThreadLocal<LuaState> every time.
    //luaStates_ a cache, so the act of ignoring its contents
    //does not cause any problems.
    copy_as_new_ptr<ThreadLocal<LuaState>> luaStates_;

    std::vector<char> compiledMainChunk_;
    std::vector<LuaModule> compiledExtraChunks_;
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
