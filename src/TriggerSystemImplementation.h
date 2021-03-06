#ifndef HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
#define HG_TRIGGER_SYSTEM_IMPLEMENTATION_H

#include "Guy.h"
#include "Box.h"
#include "TriggerData.h"
#include "ObjectPtrList.h"
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "PhysicsAffectingStuff.h"
#include "ObjectAndTime.h"
#include "mt/boost/container/vector.hpp"
#include "mt/boost/container/map.hpp"
#include "mt/std/map"
#include "OperationInterrupter.h"
#include "memory_pool.h"

#include <boost/optional.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/swap.hpp>
#include <boost/tuple/tuple.hpp>

#include <utility>

#include "multi_thread_deleter.h"

#include "Frame_fwd.h"
namespace hg {
//Moveable but non-copyable.
class TriggerFrameStateImplementation
{
    public:
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const *currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) = 0;
    
    virtual bool shouldArrive(Guy const &potentialArriver) = 0;
    virtual bool shouldArrive(Box const &potentialArriver) = 0;
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const &potentialPorter,
        bool porterActionedPortal) = 0;
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const &potentialPorter,
        bool porterActionedPortal) = 0;
    
    virtual boost::optional<Guy> mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Guy const &objectToManipulate) = 0;
    virtual boost::optional<Box> mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Box const &objectToManipulate) = 0;

    struct DepartureInformation {
        mt::std::map<Frame *, mt::std::vector<TriggerData>> triggerDepartures;
        mt::std::vector<Glitz> forwardsGlitz;
        mt::std::vector<Glitz> reverseGlitz;
        mt::std::vector<GlitzPersister> additionalGlitzPersisters;
        mt::std::vector<ObjectAndTime<Box, Frame *>> additionalBoxDepartures;
    };

    virtual DepartureInformation getDepartureInformation(
        mt::std::map<Frame *, ObjectList<Normal>> const &departures,
        Frame *currentFrame) = 0;

    virtual ~TriggerFrameStateImplementation(){}
};

class TriggerFrameState;
//Movable but non-copyable.
//Movable using the move() member function, and also with swap().
class TriggerFrameState final
{
    public:
    typedef TriggerFrameStateImplementation::DepartureInformation DepartureInformation;
    PhysicsAffectingStuff
    calculatePhysicsAffectingStuff(
        Frame const *currentFrame,
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals)
    {
        return impl->calculatePhysicsAffectingStuff(currentFrame, triggerArrivals);
    }
    template<typename ObjectT>
    bool shouldArrive(ObjectT const &potentialArriver)
    {
        return impl->shouldArrive(potentialArriver);
    }
    
    template<typename ObjectT>
    bool shouldPort(
        int responsiblePortalIndex,
        ObjectT const &potentialPorter,
        bool porterActionedPortal)
    {
        return impl->shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);
    }
    
    template<typename ObjectT>
    boost::optional<ObjectT> mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        ObjectT const &objectToManipulate)
    {
        return impl->mutateObject(responsibleMutatorIndices, objectToManipulate);
    }
    
    DepartureInformation getDepartureInformation(
        mt::std::map<Frame *, ObjectList<Normal>> const &departures,
        Frame *currentFrame)
    {
        return impl->getDepartureInformation(departures, currentFrame);
    }

    //Default constructed TriggerFrameState may not have any functions called on it,
    //but may be swapped with a TriggerFrameState that does.
    TriggerFrameState() : impl()
    {}

    explicit TriggerFrameState(TriggerFrameStateImplementation *impl) :
        impl(impl)
    {}
    TriggerFrameState(TriggerFrameState &&o) :
        impl()
    {
        swap(o);
    }
    TriggerFrameState &operator=(TriggerFrameState &&o)
    {
        swap(o);
        return *this;
    }
    void swap(TriggerFrameState &o)
    {
        boost::swap(impl, o.impl);
    }
    ~TriggerFrameState() {
    }
    private:
    std::unique_ptr<
        TriggerFrameStateImplementation,
        memory_pool_deleter<TriggerFrameStateImplementation>> impl;
};
inline void swap(TriggerFrameState &l, TriggerFrameState &r) { l.swap(r); }

class TriggerSystemImplementation
{
    public:
    virtual TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const = 0;
    virtual TriggerSystemImplementation *clone() const = 0;
    virtual ~TriggerSystemImplementation(){}
    virtual bool operator==(TriggerSystemImplementation const &o) const = 0;
    //Arbitrary number, must be distinct for each different implementation
    //(Could just as well come from a comparison of typeid addresses or something).
    //Currently:
    //DirectLuaTriggerSystem == 1000
    //SimpleConfiguredTriggerSystem == 2000
    //ComparisonTestTriggerSystem == 10000
    virtual int order_ranking() const = 0;
};
}

#endif //HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
