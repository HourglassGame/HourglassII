#ifndef HG_CONCURRENT_TIME_SET_H
#define HG_CONCURRENT_TIME_SET_H
#include <tbb/concurrent_hash_map.h>
#include <boost/range/adaptor/map.hpp>
#include <boost/range.hpp>
#include "BoostHashCompare.h"
#include <boost/move/move.hpp>
#include "Frame_fwd.h"
namespace hg {
/**
 *  Unordered set providing the following guarantees:
 *
 *  Safe to call add() concurrently with other calls to add() or remove() as long as calls
 *  to add or remove with the same argument are serialised.
 *
 *  Safe to call remove() concurrently with other calls to add() or remove() as long as calls
 *  to add or remove with the same argument are serialised.
 *
 *  Safe to concurrently read (call const member functions) from container as long as there are no concurrent writers.
 *
 *  Further guarantees...? (TODO documentation not yet complete)
 */
class ConcurrentTimeSet {
    struct Empty {};
    //hash map is being used as set, second template argument (Empty) is unused filler.
    typedef tbb::concurrent_hash_map<Frame*, Empty, BoostHashCompare<Frame*> > SetType;
public:
    ConcurrentTimeSet();
    ConcurrentTimeSet(ConcurrentTimeSet const& o) :
        set_(o.set_)
    {}
    ConcurrentTimeSet& operator=(BOOST_COPY_ASSIGN_REF(ConcurrentTimeSet) o)
    {
        return *this = ConcurrentTimeSet(o);
    }
    ConcurrentTimeSet(BOOST_RV_REF(ConcurrentTimeSet) o) :
    	set_()
    {
        set_.swap(o.set_);
   	}
    ConcurrentTimeSet& operator=(BOOST_RV_REF(ConcurrentTimeSet) o)
    {
        set_.swap(o.set_);
        return *this;
    }
    //Must never try to add or remove a particular frame concurrently,
    //only has safe concurrent access when called with different frames
    void add(Frame* toAdd);
    void remove(Frame* toRemove);
    void swap(ConcurrentTimeSet& o) {
        set_.swap(o.set_);
    }
    void clear() {
        set_.clear();
    }
    bool empty() const {
        return set_.empty();
    }
    std::size_t size() const {
        return set_.size();
    }
    typedef boost::range_iterator<boost::select_first_range<SetType> >::type iterator;
    typedef boost::range_iterator<boost::select_first_range<SetType> const>::type const_iterator;
    typedef boost::range_reference<boost::select_first_range<SetType> >::type reference;
    typedef boost::range_reference<boost::select_first_range<SetType> const>::type const_reference;
    typedef boost::range_pointer<boost::select_first_range<SetType> >::type pointer;
    typedef boost::range_pointer<boost::select_first_range<SetType> const>::type const_pointer;
    iterator begin() {
        return boost::begin(boost::adaptors::keys(set_));
    }
    const_iterator begin() const {
        return boost::begin(boost::adaptors::keys(set_));
    }
    iterator end() {
        return boost::end(boost::adaptors::keys(set_));
    }
    const_iterator end() const {
        return boost::end(boost::adaptors::keys(set_));
    }
    reference front() {
        return *begin();
    }
    const_reference front() const {
        return *begin();
    }
    reference back() {
        return *end();
    }
    const_reference back() const {
        return *end();
    }
private:
    SetType set_;
    BOOST_COPYABLE_AND_MOVABLE(ConcurrentTimeSet)
};
inline void swap(ConcurrentTimeSet& l, ConcurrentTimeSet& r)
{
    ConcurrentTimeSet temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}
#endif //HG_CONCURRENT_TIME_SET_H
