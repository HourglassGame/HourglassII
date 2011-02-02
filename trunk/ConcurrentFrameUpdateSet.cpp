#include "ConcurrentFrameUpdateSet.h"
namespace hg {
ConcurrentFrameUpdateSet::ConcurrentFrameUpdateSet() :
threadLocalMap_()
{
}
void ConcurrentFrameUpdateSet::add(const FrameUpdateSet& toAdd)
{
    threadLocalMap_.local().add(toAdd);
}
struct FrameUpdateSetCombiner
{
    FrameUpdateSetCombiner(FrameUpdateSet& taker) :
    taker_(taker)
    {
    }
    void operator()(const FrameUpdateSet& toAdd) const
    {
        taker_.add(toAdd);
    }
    FrameUpdateSet& taker_;
};
FrameUpdateSet ConcurrentFrameUpdateSet::merge()
{
    FrameUpdateSet retv;
    threadLocalMap_.combine_each(FrameUpdateSetCombiner(retv));
    return retv;
}
}
