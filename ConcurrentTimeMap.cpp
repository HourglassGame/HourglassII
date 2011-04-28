#include "ConcurrentTimeMap.h"
namespace hg {
ConcurrentTimeMap::ConcurrentTimeMap() :
        map_()
{
}
//in this implementation the comment about adding or removing a particular frame concurrently
//is not true. These operations are safe.
void ConcurrentTimeMap::add(Frame* toAdd, TimeDirection direction)
{
    MapType::accessor acc;
    if (!map_.insert(acc, MapType::value_type(toAdd, direction))) {
        acc->second = direction;
    }
}
void ConcurrentTimeMap::remove(Frame* toRemove)
{
    map_.erase(toRemove);
}
}
