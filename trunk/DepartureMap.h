#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include "FrameID.h"
#include "TimeObjectListList.h"
#include "FrameUpdateSet.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/unordered_map.hpp>

namespace hg {
    class DepartureMap {
    public:
        
        typedef ::boost::unordered_map<FrameID, TimeObjectListList> MapType;
        typedef MapType::value_type ValueType;
        //BIG privacy leak here, it is incorrect to use DepartureMap::const_iterator as anything but a forward_iterator
        //To fix make new iterator class, but that would take effort...
        typedef MapType::const_iterator const_iterator;
        
        DepartureMap() :
        map_()
        {
        }
        //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
        void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
        void addDeparture(FrameID time, TimeObjectListList departingObjects)
        {
            swap(map_[time],departingObjects);
        }
        const_iterator begin() const
        {
            return map_.begin();
        }
        const_iterator end() const
        {
            return map_.end();
        }
    private:
        MapType map_;
    };
}

#endif //HG_DEPARTURE_MAP_H
