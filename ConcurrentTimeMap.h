#ifndef HG_CONCURRENT_TIME_MAP_H
#define HG_CONCURRENT_TIME_MAP_H
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include "NewFrameID.h"
#include "TimeDirection.h"
namespace hg {
    class ConcurrentTimeMap {
        typedef boost::unordered_map<NewFrameID, TimeDirection> MapType;
        public:
        ConcurrentTimeMap();
        //Must never try to add or remove a particular frame concurrently,
        //only has safe concurrent access when each thread is calling 
        //add or remove with different Frame parameters
        void add(const NewFrameID& toAdd, TimeDirection direction);
        void remove(const NewFrameID& toRemove);
        void clear() { map_.clear(); }
        bool empty() const { return map_.empty(); }
        size_t size() const { return map_.size(); }
        typedef MapType::iterator iterator;
        typedef MapType::const_iterator const_iterator;
        iterator begin() { return map_.begin(); }
        const_iterator begin() const { return map_.begin(); }
        iterator end() { return map_.end(); }
        const_iterator end() const { return map_.end(); }
        private:
        boost::shared_mutex mutex_;
        MapType map_;
    };
}
#endif //HG_CONCURRENT_TIME_MAP_H
