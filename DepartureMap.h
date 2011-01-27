#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include "FrameID.h"
#include "TimeObjectListList.h"
#include "FrameUpdateSet.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
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
        mutex_(),
        map_()
        {
        }
        void reserve(size_t toReseve)
        {
            map_.reserve(toReseve);
        }
        void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor)
        {
            //Could remove the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
            //CBF ATM.
            //instead I do this:
            map_.reserve(toMakeSpaceFor.size());
        }
        void addDeparture(NewFrameID time, TimeObjectListList departingObjects)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
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
        boost::mutex mutex_;
        MapType map_;
    };
}

#endif //HG_DEPARTURE_MAP_H
