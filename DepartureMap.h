#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H


#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"

#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "mt/boost/container/map.hpp"
#include <boost/unordered_map.hpp>
namespace hg {
class DepartureMap {
    typedef
    boost::unordered_map<
        Frame*,
        mt::std::map<Frame*, ObjectList<Normal> >::type
    > MapType;
public:
    typedef MapType::value_type value_type;
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;
    DepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void setDeparture(Frame* time, MapType::mapped_type& departingObjects);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
private:
    MapType map_;
};
}

#endif //HG_DEPARTURE_MAP_H
