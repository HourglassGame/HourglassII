#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include "FrameID.h"
#include "TimeObjectListList.h"
#include <vector>

namespace hg {
    class DepartureMap {
    public:
        typedef ::std::pair<NewFrameID, TimeObjectListList> ValueType;
        typedef ::std::vector<ValueType> MapType;
        
        //BIG privacy leak here, it is incorrect to use DepartureMap::const_iterator as anything but a forward_iterator
        //To fix make new iterator class, but that would take effort...
        typedef MapType::const_iterator const_iterator;
        
        DepartureMap() :
        map_()
        {
        }
        void reserve(size_t toReseve)
        {
            map_.reserve(toReseve);
        }
        void addDeparture(NewFrameID time, TimeObjectListList departingObjects)
        {
            map_.push_back(ValueType(time, departingObjects));
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
