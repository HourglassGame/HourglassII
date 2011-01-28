#include "DepartureMap.h"
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
namespace hg {
void DepartureMap::makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
{
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    foreach(const FrameID& frame, toMakeSpaceFor)
    {
        map_.insert(ValueType(frame, MapType::mapped_type()));
    }
}
}
