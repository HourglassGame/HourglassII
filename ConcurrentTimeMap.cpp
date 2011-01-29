#include "ConcurrentTimeMap.h"
#include <boost/thread/locks.hpp>
namespace hg {
ConcurrentTimeMap::ConcurrentTimeMap() :
mutex_(),
map_()
{
}
/*
void ConcurrentTimeMap::add(const FrameID& toAdd, TimeDirection direction)
{
    MapType::accessor acc;
    if (!map_.insert(acc, MapType::value_type(toAdd, direction))) {
        acc->second = direction;
    }
}
void ConcurrentTimeMap::remove(const FrameID& toRemove)
{
    map_.erase(toRemove);
}*/

void ConcurrentTimeMap::add(const FrameID& toAdd, TimeDirection direction)
{
    boost::shared_lock<boost::shared_mutex> shared(mutex_);
    iterator it(map_.find(toAdd));
    if (it == map_.end()) {
        shared.unlock();
        boost::lock_guard<boost::shared_mutex> unique(mutex_);
        map_.insert(MapType::value_type(toAdd, direction));
    }
    else if (it->second != direction){
        it->second != direction;
        shared.unlock();
        boost::lock_guard<boost::shared_mutex> unique(mutex_);
        map_[toAdd] = direction;
    }
}
void ConcurrentTimeMap::remove(const FrameID& toRemove)
{
    boost::shared_lock<boost::shared_mutex> shared(mutex_);
    if (map_.find(toRemove) != map_.end()) {
        //must leave reader's lock section to aviod deadlock if two threads simultaneously need to write
        shared.unlock();
        //Cannot use iterator from earlier, as may be invalidated by another write.
        //However, finding it again is safe because it is against the interface of these functions to 
        //call them with the same FrameID concurrently (so this element could not have been removed by another thread)
        boost::lock_guard<boost::shared_mutex> unique(mutex_);
        map_.quick_erase(map_.find(toRemove));
    }
}
}