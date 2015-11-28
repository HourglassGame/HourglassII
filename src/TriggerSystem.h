#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "clone_ptr.h"
#include "unique_ptr.h"
namespace hg
{
class TriggerSystem {
    clone_ptr<TriggerSystemImplementation> impl;
    typedef
        std::tuple<
        decltype(impl->order_ranking()),
        TriggerSystemImplementation const &>
        comparison_tuple_type;
    comparison_tuple_type comparison_tuple() const {
        return comparison_tuple_type(impl->order_ranking(), *impl);
    }
public:
    //default constructed triggers system to enable moving.
    //a TriggerSystem without a TriggerSystemImplementation
    //is only useful for swapping into a TriggerSystem with an implementation
    //as an O(1) operation.
    TriggerSystem() : impl() {}
    //Takes ownership of impl
    template<typename TriggerSystemImplementation>
    TriggerSystem(unique_ptr<TriggerSystemImplementation> impl) :
        impl(impl.release())
    {}

    void swap(TriggerSystem &o) {
        impl.swap(o.impl);
    }
    //In a break from the usual OperationInterrupter semantics,
    //the interrupter which is passed to this function is used for the entire lifetime
    //of the returned TriggerFrameState.
    TriggerFrameState getFrameState(OperationInterrupter &interrupter) const
    {
        return impl.get().getFrameState(interrupter);
    }
    bool operator==(TriggerSystem const &o) const {
        return comparison_tuple() == o.comparison_tuple();
    }
};
inline void swap(TriggerSystem &l, TriggerSystem &r)
{
    l.swap(r);
}
}
#endif //HG_TRIGGER_SYSTEM_H
