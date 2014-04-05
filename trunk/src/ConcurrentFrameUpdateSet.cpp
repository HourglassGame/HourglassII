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

FrameUpdateSet ConcurrentFrameUpdateSet::merge()
{
    FrameUpdateSet retv;
    threadLocalMap_.combine_each([&](FrameUpdateSet const &toAdd){retv.add(toAdd);});
    return retv;
}
}
