#include "ConcurrentTimeMap.h"
#include <boost/thread/locks.hpp>
namespace hg {
ConcurrentTimeMap::ConcurrentTimeMap() :
mutex_(),
map_()
{
}
void ConcurrentTimeMap::add(const NewFrameID& toAdd, TimeDirection direction)
{
    {
        boost::upgrade_lock<boost::shared_mutex> shared(mutex_);
        if (map_.find(toAdd) == map_.end()) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(shared);
            map_.insert(MapType::value_type(toAdd, direction));
        }
    }
}
void ConcurrentTimeMap::remove(const NewFrameID& toRemove)
{
    {
        boost::upgrade_lock<boost::shared_mutex> shared(mutex_);
        iterator it(map_.find(toRemove));
        if (it != map_.end()) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(shared);
            map_.quick_erase(it);
        }
    }
}
}