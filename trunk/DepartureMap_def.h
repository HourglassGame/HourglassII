#include "DepartureMap.h"
#include "FrameUpdateSet.h"
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
namespace hg {
template<typename DepartureListT>
DepartureMap<DepartureListT>::DepartureMap() :
        map_()
{
}
template<typename DepartureListT>
void DepartureMap<DepartureListT>::makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
{
    map_.rehash(toMakeSpaceFor.size());
    //removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
    foreach(Frame* frame, toMakeSpaceFor)
    {
        map_.insert(value_type(frame, typename MapType::mapped_type()));
    }
}
template<typename DepartureListT>
void DepartureMap<DepartureListT>::setDeparture(Frame* time, std::map<Frame*, DepartureListT>& departingObjects)
{
    map_[time].swap(departingObjects);
}
template<typename DepartureListT>
typename DepartureMap<DepartureListT>::iterator DepartureMap<DepartureListT>::begin()
{
    return map_.begin();
}
template<typename DepartureListT>
typename DepartureMap<DepartureListT>::iterator DepartureMap<DepartureListT>::end()
{
    return map_.end();
}
template<typename DepartureListT>
typename DepartureMap<DepartureListT>::const_iterator DepartureMap<DepartureListT>::begin() const
{
    return map_.begin();
}
template<typename DepartureListT>
typename DepartureMap<DepartureListT>::const_iterator DepartureMap<DepartureListT>::end() const
{
    return map_.end();
}
}
