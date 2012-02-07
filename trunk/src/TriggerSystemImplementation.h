#ifndef HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
#define HG_TRIGGER_SYSTEM_IMPLEMENTATION_H

#include "Guy.h"
#include "Box.h"
#include "TriggerData.h"
#include "ObjectPtrList.h"
#include "RetardedNotActuallyAGlitzGlitz.h"
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "PhysicsAffectingStuff.h"
#include "ObjectAndTime.h"
#include "mt/boost/container/vector.hpp"
#include "mt/boost/container/map.hpp"
#include "mt/std/map"
#include "OperationInterrupter.h"

#include <boost/optional.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/swap.hpp>
#include <boost/tuple/tuple.hpp>

#include <utility>
#include "Frame_fwd.h"
namespace hg {
//Moveable but non-copyable.
class TriggerFrameStateImplementation
{
    public:
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const* currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr>::type const> const& triggerArrivals/*,
            OperationInterrupter& interrupter*/) = 0;
    
    virtual bool shouldArrive(Guy const& potentialArriver/*,OperationInterrupter& interrupter*/) = 0;
    virtual bool shouldArrive(Box const& potentialArriver/*,OperationInterrupter& interrupter*/) = 0;
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter,
        bool porterActionedPortal/*,
        OperationInterrupter& interrupter*/) = 0;
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter,
        bool porterActionedPortal/*,
        OperationInterrupter& interrupter*/) = 0;
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Guy const& objectToManipulate/*,
        OperationInterrupter& interrupter*/) = 0;
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Box const& objectToManipulate/*,
        OperationInterrupter& interrupter*/) = 0;

    struct DepartureInformation {
        DepartureInformation(
            mt::std::map<Frame*, mt::std::vector<TriggerData>::type>::type const& ntriggerDepartures,
            mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type const& nbackgroundGlitz,
            mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type const& nforegroundGlitz,
            mt::std::vector<ObjectAndTime<Box, Frame*> >::type const& nadditionalBoxDepartures):
                triggerDepartures(ntriggerDepartures),
                backgroundGlitz(nbackgroundGlitz),
                foregroundGlitz(nforegroundGlitz),
                additionalBoxDepartures(nadditionalBoxDepartures) {}
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type>::type triggerDepartures;
		mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type backgroundGlitz;
        mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type foregroundGlitz;
		mt::std::vector<ObjectAndTime<Box, Frame*> >::type additionalBoxDepartures;
    };

    virtual DepartureInformation getDepartureInformation(
        mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame/*,
        OperationInterrupter& interrupter*/) = 0;

    virtual ~TriggerFrameStateImplementation(){}
};

class TriggerFrameState;
//Movable but non-copyable.
//Movable using the move() member function, and also with swap().
class TriggerFrameState
{
    public:
    typedef TriggerFrameStateImplementation::DepartureInformation DepartureInformation;
    PhysicsAffectingStuff
    calculatePhysicsAffectingStuff(
        Frame const* currentFrame,
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr>::type const> const& triggerArrivals/*,
            OperationInterrupter& interrupter*/)
    {
        return impl_->calculatePhysicsAffectingStuff(currentFrame, triggerArrivals/*, interrupter*/);
    }
    template<typename ObjectT>
    bool shouldArrive(ObjectT const& potentialArriver/*,OperationInterrupter& interrupter*/)
    {
        return impl_->shouldArrive(potentialArriver/*, interrupter*/);
    }
    
    template<typename ObjectT>
    bool shouldPort(
        int responsiblePortalIndex,
        ObjectT const& potentialPorter,
        bool porterActionedPortal/*,
        OperationInterrupter& interrupter*/)
    {
        return impl_->shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal/*, interrupter*/);
    }
    
    template<typename ObjectT>
    boost::optional<ObjectT> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        ObjectT const& objectToManipulate/*,
        OperationInterrupter& interrupter*/)
    {
        return impl_->mutateObject(responsibleMutatorIndices, objectToManipulate/*, interrupter*/);
    }
    
    DepartureInformation getDepartureInformation(
            mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
            Frame* currentFrame/*,
            OperationInterrupter& interrupter*/)
    {
        return impl_->getDepartureInformation(departures, currentFrame/*, interrupter*/);
    }

    //Default constructed TriggerFrameState may not have any functions called on it,
    //but may be swapped with a TriggerFrameState that does.
    TriggerFrameState() : impl_(0)
    {}

    //Takes ownership of impl.
    //precondition:
    //  The following must release impl:
    //  if (impl) {
    //      impl->~TriggerFrameStateImplementation();
    //      multi_thread_free(impl);
    //  }
    //THIS MEANS THAT impl MUST HAVE BEEN ALLOCATED IN A SPECIAL WAY!!
    explicit TriggerFrameState(TriggerFrameStateImplementation* impl) :
        impl_(impl)
    {}
    TriggerFrameState(BOOST_RV_REF(TriggerFrameState) o) :
        impl_(0)
    {
        swap(o);
    }
    TriggerFrameState& operator=(BOOST_RV_REF(TriggerFrameState) o)
    {
        swap(o);
        return *this;
    }
    void swap(TriggerFrameState& o)
    {
        boost::swap(impl_, o.impl_);
    }
    ~TriggerFrameState() {
        if (impl_) {
            impl_->~TriggerFrameStateImplementation();
            multi_thread_free(impl_);
        }
    }
    private:
    TriggerFrameStateImplementation* impl_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(TriggerFrameState)
};
inline void swap(TriggerFrameState& l, TriggerFrameState& r) { l.swap(r); }

class TriggerSystemImplementation
{
    public:
    virtual TriggerFrameState getFrameState(OperationInterrupter& interrupter) const = 0;
    virtual TriggerSystemImplementation* clone() const = 0;
    virtual ~TriggerSystemImplementation(){}
};
}

#endif //HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
