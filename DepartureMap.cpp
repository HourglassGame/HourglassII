#include "DepartureMap.h"
#include "FrameUpdateSet.h"
#include <boost/foreach.hpp>

#define foreach BOOST_FOREACH
namespace hg {
DepartureMap::DepartureMap() :
        map_()
{
}

void DepartureMap::makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
{
    map_.rehash(toMakeSpaceFor.size());
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    foreach(Frame* frame, toMakeSpaceFor)
    {
        map_.insert(value_type(frame, MapType::mapped_type()));
    }
}
void DepartureMap::setDeparture(Frame* time, std::map<Frame*, ObjectList<Normal> >& departingObjects)
{
    map_[time].swap(departingObjects);
}

DepartureMap::iterator DepartureMap::begin()
{
    return map_.begin();
}
DepartureMap::iterator DepartureMap::end()
{
    return map_.end();
}
DepartureMap::const_iterator DepartureMap::begin() const
{
    return map_.begin();
}
DepartureMap::const_iterator DepartureMap::end() const
{
    return map_.end();
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
