#include "ConcurrentFrameUpdateSet.h"

namespace hg {
ConcurrentFrameUpdateSet::ConcurrentFrameUpdateSet() :
        threadLocalMap_()
{
}
void ConcurrentFrameUpdateSet::add(FrameUpdateSet const &toAdd)
{
    threadLocalMap_.local().add(toAdd);
}
struct FrameUpdateSetCombiner
{
    FrameUpdateSetCombiner(FrameUpdateSet &taker) :
            taker_(taker)
    {
    }
    void operator()(FrameUpdateSet const &toAdd) const
    {
        taker_.add(toAdd);
    }
    FrameUpdateSet &taker_;
};

FrameUpdateSet ConcurrentFrameUpdateSet::merge()
{
    FrameUpdateSet retv;
    threadLocalMap_.combine_each(FrameUpdateSetCombiner(retv));
    return retv;
}
}
