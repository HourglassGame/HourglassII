#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "clone_ptr.h"
#include "move.h"
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
    TriggerSystem(TriggerSystemImplementation* impl) :
        impl_(impl)
    {}
    TriggerSystem(TriggerSystem const& other) : impl_(other.impl_) {}
    TriggerSystem& operator=(TriggerSystem const& other)
    {
        impl_ = other.impl_;
        return *this;
    }
    TriggerSystem(TriggerSystem&& other) :
        impl_(hg::move(other.impl_))
    {
    }
    TriggerSystem& operator=(TriggerSystem&& other)
    {
        impl_ = hg::move(other.impl_);
        return *this;
    }

    void swap(TriggerSystem& other) {
        impl_.swap(other.impl_);
    }
    
    TriggerFrameState getFrameState() const
    {
        return impl_.get().getFrameState();
    }
private:
    clone_ptr<TriggerSystemImplementation> impl_;
};
inline void swap(TriggerSystem& l, TriggerSystem& r)
{
    l.swap(r);
}
}
#endif //HG_TRIGGER_SYSTEM_H
