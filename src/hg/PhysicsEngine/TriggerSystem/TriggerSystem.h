#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "hg/Util/clone_ptr.h"
#include <memory>
namespace hg
{
class TriggerSystem final {
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
    explicit TriggerSystem() : impl() {}
    //Takes ownership of impl
    template<typename TriggerSystemImplementation>
    explicit TriggerSystem(std::unique_ptr<TriggerSystemImplementation> impl) :
        impl(impl.release())
    {}

    void swap(TriggerSystem &o) noexcept {
        impl.swap(o.impl);
    }
    //In a break from the usual OperationInterrupter semantics,
    //the interrupter which is passed to this function is used for the entire lifetime
    //of the returned TriggerFrameState.
    TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const
    {
        return impl.get().getFrameState(pool, interrupter);
    }
    bool operator==(TriggerSystem const &o) const {
        return comparison_tuple() == o.comparison_tuple();
    }
};
inline void swap(TriggerSystem &l, TriggerSystem &r) noexcept
{
    l.swap(r);
}
}
#endif //HG_TRIGGER_SYSTEM_H
