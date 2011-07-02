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

EditDepartureMap::EditDepartureMap() :
        map_()
{
}

void EditDepartureMap::makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
{
    map_.rehash(toMakeSpaceFor.size());
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    foreach(Frame* frame, toMakeSpaceFor)
    {
        map_.insert(value_type(frame, MapType::mapped_type()));
    }
}
void EditDepartureMap::setDeparture(Frame* time, std::map<Frame*, ObjectList<FirstEdit> >& departingObjects)
{
    map_[time].swap(departingObjects);
}

EditDepartureMap::iterator EditDepartureMap::begin()
{
    return map_.begin();
}
EditDepartureMap::iterator EditDepartureMap::end()
{
    return map_.end();
}
EditDepartureMap::const_iterator EditDepartureMap::begin() const
{
    return map_.begin();
}
EditDepartureMap::const_iterator EditDepartureMap::end() const
{
    return map_.end();
}
EditDepartureMap::const_iterator EditDepartureMap::cbegin() const
{
    return begin();
}
EditDepartureMap::const_iterator EditDepartureMap::cend() const
{
    return end();
}


RawDepartureMap::RawDepartureMap() :
        map_()
{
}

void RawDepartureMap::makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
{
    map_.rehash(toMakeSpaceFor.size());
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    foreach(Frame* frame, toMakeSpaceFor)
    {
        map_.insert(value_type(frame, MapType::mapped_type()));
    }
}
void RawDepartureMap::setDeparture(Frame* time, std::map<Frame*, ObjectList<Normal> >& departingObjects)
{
    map_[time].swap(departingObjects);
}

RawDepartureMap::iterator RawDepartureMap::begin()
{
    return map_.begin();
}
RawDepartureMap::iterator RawDepartureMap::end()
{
    return map_.end();
}
RawDepartureMap::const_iterator RawDepartureMap::begin() const
{
    return map_.begin();
}
RawDepartureMap::const_iterator RawDepartureMap::end() const
{
    return map_.end();
}
RawDepartureMap::const_iterator RawDepartureMap::cbegin() const
{
    return begin();
}
RawDepartureMap::const_iterator RawDepartureMap::cend() const
{
    return end();
}

}
