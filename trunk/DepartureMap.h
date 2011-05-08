#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include "ObjectList.h"
#include <boost/unordered_map.hpp>
#include <map>

#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"

namespace hg {
class DepartureMap {
public:
    typedef boost::unordered_map<Frame*, std::map<Frame*, ObjectList> > MapType;
    typedef MapType::value_type ValueType;
    typedef MapType::value_type         value_type;
    typedef MapType::pointer            pointer;
    typedef MapType::const_pointer      const_pointer;
    typedef MapType::reference          reference;
    typedef MapType::const_reference    const_reference;
    typedef MapType::size_type          size_type;
    typedef MapType::difference_type    difference_type;
    typedef MapType::iterator           iterator;
    typedef MapType::const_iterator     const_iterator;
    DepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void addDeparture(Frame* time, std::map<Frame*, ObjectList> departingObjects);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
private:
    MapType map_;
};
}

#endif //HG_DEPARTURE_MAP_H
