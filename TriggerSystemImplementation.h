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
#include "mt/std/vector"
#include "mt/std/map"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/swap.hpp>

#include <utility>
#include "Frame_fwd.h"
namespace hg {
//Moveable but non-copyable.
class TriggerFrameStateImplementation
{
    public:
    virtual PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals) = 0;
    
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Guy const& potentialPorter) = 0;
    virtual bool shouldPort(
        int responsiblePortalIndex,
        Box const& potentialPorter) = 0;
        
    virtual bool shouldPickup(
        int responsiblePickupIndex,
        Guy const& potentialPickuper) = 0;
    virtual bool shouldPickup(
        int responsiblePickupIndex,
        Box const& potentialPickuper) = 0;
        
    virtual bool shouldDie(
        int responsibleKillerIndex,
        Guy const& potentialDier) = 0;
    virtual bool shouldDie(
        int responsibleKillerIndex,
        Box const& potentialDier) = 0;
    
    virtual std::pair<
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
        mt::std::vector<RectangleGlitz>::type
    > 
    getTriggerDeparturesAndGlitz(
        mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame) = 0;

    virtual ~TriggerFrameStateImplementation(){}
};

class TriggerFrameState;
//Serves a similar purpose to auto_ptr_ref, allows TriggerFrameStates to be returned from functions
struct TriggerFrameStateMove_t
{
    TriggerFrameState& tfs_;
    explicit TriggerFrameStateMove_t(TriggerFrameState& tfs):
        tfs_(tfs)
    {}

    TriggerFrameState& operator*() const
    {
        return tfs_;
    }

    TriggerFrameState* operator->() const
    {
        return &tfs_;
    }
private:
    TriggerFrameStateMove_t& operator=(TriggerFrameStateMove_t&);
};

//Moveable but non-copyable.
//Moveable using the move() member function, and also with swap().
class TriggerFrameState
{
    public:
    PhysicsAffectingStuff
    calculatePhysicsAffectingStuff(
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::std::vector<TriggerDataConstPtr>::type const> const& triggerArrivals)
    {
        return impl_->calculatePhysicsAffectingStuff(triggerArrivals);
    }
    template<typename ObjectT>
    bool shouldPort(
        int responsiblePortalIndex,
        ObjectT const& potentialPorter)
    {
        return impl_->shouldPort(responsiblePortalIndex, potentialPorter);
    }
    template<typename ObjectT>
    bool shouldPickup(
        int responsiblePickupIndex,
        ObjectT const& potentialPickuper)
    {
        return impl_->shouldPickup(responsiblePickupIndex, potentialPickuper);
    }
    template<typename ObjectT>
    bool shouldDie(
        int responsibleKillerIndex,
        ObjectT const& potentialDier)
    {
        return impl_->shouldDie(responsibleKillerIndex, potentialDier);
    }
    
    std::pair<
        mt::std::map<Frame*, mt::std::vector<TriggerData>::type >::type,
        mt::std::vector<RectangleGlitz>::type
    > 
        getTriggerDeparturesAndGlitz(
            mt::std::map<Frame*, ObjectList<Normal> >::type const& departures,
            Frame* currentFrame)
    {
        return impl_->getTriggerDeparturesAndGlitz(departures, currentFrame);
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
    TriggerFrameState(TriggerFrameStateMove_t mover) :
        impl_(0)
    {
        this->swap(*mover);
    }
    TriggerFrameState& operator=(TriggerFrameStateMove_t mover)
    {
        this->swap(*mover);
        return *this;
    }
    TriggerFrameState(TriggerFrameState& other) :
        impl_(0)
    {
        swap(other);
    }
    TriggerFrameState& operator=(TriggerFrameState& other)
    {
        swap(other);
        return *this;
    }
    TriggerFrameStateMove_t move()
    {
        return TriggerFrameStateMove_t(*this);
    }
    operator TriggerFrameStateMove_t()
    {
        return move();
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
