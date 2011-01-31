#include "ConcurrentFrameUpdateSet.h"
#include <boost/thread/locks.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
namespace hg {
ConcurrentFrameUpdateSet::ConcurrentFrameUpdateSet() :
threadLocalMap_(),
mutex_()
{
}
void ConcurrentFrameUpdateSet::add(const FrameUpdateSet& toAdd)
{
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    const boost::thread::id threadID(boost::this_thread::get_id());
    MapType::iterator it(threadLocalMap_.find(threadID));
    if (it != threadLocalMap_.end()) {
        it->second.add(toAdd);
    }
    else {
        lock.unlock();
        boost::lock_guard<boost::shared_mutex> lock(mutex_);
        threadLocalMap_.insert(MapType::value_type(threadID, toAdd));
    }
}
FrameUpdateSet ConcurrentFrameUpdateSet::merge()
{
    FrameUpdateSet retv;
    foreach(const MapType::value_type& value, threadLocalMap_)
    {
        retv.add(value.second);
    }
    return retv;
}
}
