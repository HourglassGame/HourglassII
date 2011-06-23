#ifndef HG_CONCURRENT_TIME_MAP_H
#define HG_CONCURRENT_TIME_MAP_H
#include <tbb/concurrent_hash_map.h>
#include "BoostHashCompare.h"
#include "Frame.h"
#include "TimeDirection.h"
namespace hg {
class ConcurrentTimeMap {
    typedef
    tbb::concurrent_hash_map
    <
        Frame*,
        TimeDirection,
        BoostHashCompare<Frame*>
    > MapType;
public:
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;
    
    ConcurrentTimeMap();
    //Must never try to add or remove a particular frame concurrently,
    //only has safe concurrent access when each thread is calling
    //add or remove with different Frame parameters
    void add(Frame* toAdd, TimeDirection direction);
    void remove(Frame* toRemove);
    
    void clear()                 { map_.clear(); }
    bool empty()           const { return map_.empty(); }
    std::size_t size()     const { return map_.size(); }
    iterator begin()             { return map_.begin(); }
    const_iterator begin() const { return map_.begin(); }
    iterator end()               { return map_.end(); }
    const_iterator end()   const { return map_.end(); }
private:
    MapType map_;
};
}
#endif //HG_CONCURRENT_TIME_MAP_H
