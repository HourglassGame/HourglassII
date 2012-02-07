#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "clone_ptr.h"
#include "unique_ptr.h"
namespace hg
{
class TriggerSystem {
public:
    //default constructed triggers system to enable moving.
    //a TriggerSystem without a TriggerSystemImplementation
    //is only useful for swapping into a TriggerSystem with an implementation
    //as an O(1) operation.
    TriggerSystem() : impl_() {}
    //Takes ownership of impl
    template<typename TriggerSystemImplementation>
    TriggerSystem(unique_ptr<TriggerSystemImplementation> impl) :
        impl_(impl.release())
    {}
    TriggerSystem(TriggerSystem const& o) : impl_(o.impl_) {}
    TriggerSystem& operator=(BOOST_COPY_ASSIGN_REF(TriggerSystem) o)
    {
        impl_ = o.impl_;
        return *this;
    }
    TriggerSystem(BOOST_RV_REF(TriggerSystem) o) :
        impl_(boost::move(o.impl_))
    {
    }
    TriggerSystem& operator=(BOOST_RV_REF(TriggerSystem) o)
    {
        impl_ = boost::move(o.impl_);
        return *this;
    }

    void swap(TriggerSystem& o) {
        impl_.swap(o.impl_);
    }
    
    TriggerFrameState getFrameState(OperationInterrupter& interrupter) const
    {
        return impl_.get().getFrameState(interrupter);
    }
private:
    clone_ptr<TriggerSystemImplementation> impl_;
    BOOST_COPYABLE_AND_MOVABLE(TriggerSystem)
};
inline void swap(TriggerSystem& l, TriggerSystem& r)
{
    l.swap(r);
}
}
#endif //HG_TRIGGER_SYSTEM_H
