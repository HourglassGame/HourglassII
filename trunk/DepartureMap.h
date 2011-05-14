#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include <boost/unordered_map.hpp>
#include <map>

#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"

namespace hg {
template<typename DepartureListT>
class DepartureMap {
    typedef typename boost::unordered_map<Frame*, typename std::map<Frame*, DepartureListT> > MapType;
public:
    typedef typename MapType::value_type         value_type;
    typedef typename MapType::pointer            pointer;
    typedef typename MapType::const_pointer      const_pointer;
    typedef typename MapType::reference          reference;
    typedef typename MapType::const_reference    const_reference;
    typedef typename MapType::size_type          size_type;
    typedef typename MapType::difference_type    difference_type;
    typedef typename MapType::iterator           iterator;
    typedef typename MapType::const_iterator     const_iterator;
    DepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void setDeparture(Frame* time, std::map<Frame*, DepartureListT>& departingObjects);
    
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
private:
    MapType map_;
};
}

#endif //HG_DEPARTURE_MAP_H
