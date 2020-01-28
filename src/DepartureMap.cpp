#include "DepartureMap.h"
#include "FrameUpdateSet.h"
#include <utility>
namespace hg {
void DepartureMap::makeSpaceFor(FrameUpdateSet const &toMakeSpaceFor)
{
    map.rehash(toMakeSpaceFor.size());
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    for (Frame *frame: toMakeSpaceFor)
    {
        map.insert(value_type(frame, MapType::mapped_type()));
    }
}
void DepartureMap::setDeparture(Frame *frame, MapType::mapped_type &&departingObjects)
{
    map.find(frame)->second = std::move(departingObjects);
}

unsigned DepartureMap::size() const
{
    return map.size();
}
DepartureMap::iterator DepartureMap::begin()
{
    return map.begin();
}
DepartureMap::iterator DepartureMap::end()
{
    return map.end();
}
DepartureMap::const_iterator DepartureMap::begin() const
{
    return map.begin();
}
DepartureMap::const_iterator DepartureMap::end() const
{
    return map.end();
}
DepartureMap::const_iterator DepartureMap::cbegin() const
{
    return begin();
}
DepartureMap::const_iterator DepartureMap::cend() const
{
    return end();
}
}
