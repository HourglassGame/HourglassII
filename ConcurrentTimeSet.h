#ifndef HG_CONCURRENT_TIME_SET_H
#define HG_CONCURRENT_TIME_SET_H
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_set.hpp>
#include "NewFrameID.h"
namespace hg {
    class ConcurrentTimeSet {
        typedef boost::unordered_set<NewFrameID> SetType;
        public:
        ConcurrentTimeSet();
        //Must never try to add or remove a particular frame concurrently,
        //only has safe concurrent access when called with different frames
        void add(const NewFrameID& toAdd);
        void remove(const NewFrameID& toRemove);
        void clear() { set_.clear(); }
        bool empty() const { return set_.empty(); }
        size_t size() const { return set_.size(); }
        typedef SetType::iterator iterator;
        typedef SetType::const_iterator const_iterator;
        iterator begin() { return set_.begin(); }
        const_iterator begin() const { return set_.begin(); }
        iterator end() { return set_.end(); }
        const_iterator end() const { return set_.end(); }
        private:
        boost::shared_mutex mutex_;
        SetType set_;
    };
}
#endif //HG_CONCURRENT_TIME_SET_H
