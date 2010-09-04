#include "FrameUpdateSet.h"
using namespace ::hg;

FrameUpdateSet::FrameUpdateSet() :
updateSet()
{
}

void FrameUpdateSet::addFrame(FrameID frame)
{
    updateSet.insert(frame);
}

FrameUpdateSet::const_iterator FrameUpdateSet::begin() const
{
    return updateSet.begin();
}

FrameUpdateSet::const_iterator FrameUpdateSet::end() const
{
    return updateSet.end();
}

void FrameUpdateSet::add(const FrameUpdateSet& other)
{
    updateSet.insert(other.updateSet.begin(), other.updateSet.end());
}
