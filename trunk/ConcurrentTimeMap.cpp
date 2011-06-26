#include "ConcurrentTimeMap.h"
namespace hg {
ConcurrentTimeMap::ConcurrentTimeMap() :
        map_()
{
}

void ConcurrentTimeMap::add(Frame* toAdd, TimeDirection direction)
{
//In this implementation the comment about adding
//or removing a particular frame concurrently
//is not true. These operations are safe.
    MapType::accessor acc;
    if (!map_.insert(acc, MapType::value_type(toAdd, direction))) {
        acc->second = direction;
    }
}
void ConcurrentTimeMap::remove(Frame const* toRemove)
{
    //const_cast well defined and safe as toRemove never written through,
    //and removing a frame* does not modify the underlying frame
    map_.erase(const_cast<Frame*>(toRemove));
}
}
