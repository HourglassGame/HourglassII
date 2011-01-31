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
        
        typedef ::boost::unordered_map<Frame*, std::map<Frame*, ObjectList> > MapType;
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
        DepartureMap() :
        map_()
        {
        }
        //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
        void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
        void addDeparture(Frame* time, std::map<Frame*, ObjectList> departingObjects)
        {
            swap(map_[time],departingObjects);
        }
        iterator begin()
        {
            return map_.begin();
        }
        iterator end()
        {
            return map_.end();
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
