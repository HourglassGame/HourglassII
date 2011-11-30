#ifndef HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
#define HG_TRIGGER_SYSTEM_IMPLEMENTATION_H

#include "Guy.h"
#include "Box.h"
#include "TriggerData.h"
#include "ObjectPtrList.h"
#include "RectangleGlitz.h"
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "PhysicsAffectingStuff.h"
#include "ObjectAndTime.h"
#include "mt/std/vector"
#include "mt/std/map"

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
                mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals) = 0;
    
    virtual bool shouldArrive(Guy const& potentialArriver) = 0;
    virtual bool shouldArrive(Box const& potentialArriver) = 0;
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter,
        bool porterActionedPortal) = 0;
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter,
        bool porterActionedPortal) = 0;
    
    virtual boost::optional<Guy> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Guy const& objectToManipulate) = 0;
    virtual boost::optional<Box> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        Box const& objectToManipulate) = 0;
    
    virtual boost::tuple<
		mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
		mt::std::vector<RectangleGlitz>::type,
		mt::std::vector<ObjectAndTime<Box> >::type
	>
    getDepartureInformation(
        mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame) = 0;

    virtual ~TriggerFrameStateImplementation(){}
};

class TriggerFrameState;
//Movable but non-copyable.
//Movable using the move() member function, and also with swap().
class TriggerFrameState
{
    public:
    PhysicsAffectingStuff
    calculatePhysicsAffectingStuff(
        Frame const* currentFrame,
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals)
    {
        return impl_->calculatePhysicsAffectingStuff(currentFrame, triggerArrivals);
    }
    template<typename ObjectT>
    bool shouldArrive(ObjectT const& potentialArriver)
    {
        return impl_->shouldArrive(potentialArriver);
    }
    
    template<typename ObjectT>
    bool shouldPort(
        int responsiblePortalIndex,
        ObjectT const& potentialPorter,
        bool porterActionedPortal)
    {
        return impl_->shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);
    }
    
    template<typename ObjectT>
    boost::optional<ObjectT> mutateObject(
        mt::std::vector<int>::type const& responsibleMutatorIndices,
        ObjectT const& objectToManipulate)
    {
        return impl_->mutateObject(responsibleMutatorIndices, objectToManipulate);
    }
    
    boost::tuple<
		mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
		mt::std::vector<RectangleGlitz>::type,
		mt::std::vector<ObjectAndTime<Box> >::type
	>
        getDepartureInformation(
            mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
            Frame* currentFrame)
    {
        return impl_->getDepartureInformation(departures, currentFrame);
    }

    //Default constructed TriggerFrameState may not have any functions called on it,
    //but may be swapped with a TriggerFrameState that does.
    TriggerFrameState() : impl_(0)
    {}

    //Takes ownership of impl
    //pre:
    //if (impl) {
    //      impl->~TriggerFrameStateImplementation();
    //      multi_thread_free(impl);
    //}
    //must release impl.
    //THIS MEANS THAT impl MUST HAVE BEEN ALLOCATED IN A SPECIAL WAY!!
    explicit TriggerFrameState(TriggerFrameStateImplementation* impl) :
        impl_(impl)
    {}
    TriggerFrameState(TriggerFrameState&& other) :
        impl_(0)
    {
        swap(other);
    }
    TriggerFrameState& operator=(TriggerFrameState&& other)
    {
        swap(other);
        return *this;
    }
    void swap(TriggerFrameState& other)
    {
        boost::swap(impl_, other.impl_);
    }
    ~TriggerFrameState() {
        if (impl_) {
            impl_->~TriggerFrameStateImplementation();
            multi_thread_free(impl_);
        }
    }
    private:
    TriggerFrameStateImplementation* impl_;
};
inline void swap(TriggerFrameState& l, TriggerFrameState& r) { l.swap(r); }

class TriggerSystemImplementation
{
    public:
    virtual TriggerFrameState getFrameState() const = 0;
    virtual TriggerSystemImplementation* clone() const = 0;
    virtual ~TriggerSystemImplementation(){}
};
TriggerSystemImplementation* new_clone(TriggerSystemImplementation const& toClone);
void delete_clone(TriggerSystemImplementation* clone);
inline TriggerSystemImplementation* new_clone(TriggerSystemImplementation const& toClone) {
    return toClone.clone();
}
inline void delete_clone(TriggerSystemImplementation* clone) {
    delete clone;
}
}

#endif //HG_TRIGGER_SYSTEM_IMPLEMENTATION_H
