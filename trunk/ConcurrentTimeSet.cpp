#include "ConcurrentTimeSet.h"
#include <boost/thread/locks.hpp>
namespace hg {
ConcurrentTimeSet::ConcurrentTimeSet() :
mutex_(),
set_()
{
}
void ConcurrentTimeSet::add(const NewFrameID& toAdd)
{
    {
        boost::upgrade_lock<boost::shared_mutex> shared(mutex_);
        if (set_.find(toAdd) == set_.end()) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(shared);
            set_.insert(toAdd);
        }
    }
}
void ConcurrentTimeSet::remove(const NewFrameID& toRemove)
{
    {
        boost::upgrade_lock<boost::shared_mutex> shared(mutex_);
        iterator it(set_.find(toRemove));
        if (it != set_.end()) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(shared);
            set_.quick_erase(it);
        }
    }
}
}