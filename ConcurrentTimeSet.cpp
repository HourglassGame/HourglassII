#include "ConcurrentTimeSet.h"
//#include <boost/thread/locks.hpp>
namespace hg {
ConcurrentTimeSet::ConcurrentTimeSet() :
set_()
{
}
void ConcurrentTimeSet::add(Frame* toAdd)
{
    set_.insert(SetType::value_type(toAdd, 0));
}
void ConcurrentTimeSet::remove(Frame* toRemove)
{
    set_.erase(toRemove);
}

/*
ConcurrentTimeSet::ConcurrentTimeSet() :
mutex_(),
set_()
{
}
void ConcurrentTimeSet::add(const FrameID& toAdd)
{
    {
        boost::shared_lock<boost::shared_mutex> shared(mutex_);
        if (set_.find(toAdd) == set_.end()) {
            shared.unlock();
            boost::lock_guard<boost::shared_mutex> unique(mutex_);
            set_.insert(toAdd);
        }
    }
}
void ConcurrentTimeSet::remove(const FrameID& toRemove)
{
    boost::shared_lock<boost::shared_mutex> shared(mutex_);
    if (set_.find(toRemove) != set_.end()) {
        shared.unlock();
        boost::lock_guard<boost::shared_mutex> unique(mutex_);
        set_.quick_erase(set_.find(toRemove));
    }
}*/
}