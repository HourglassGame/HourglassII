#ifndef HG_CONCURRENT_TIME_SET_H
#define HG_CONCURRENT_TIME_SET_H
#include <tbb/concurrent_hash_map.h>
#include <boost/range/adaptor/map.hpp>
#include <boost/range.hpp>
#include "hg/Util/StdHashCompare.h"
#include <utility>
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
class ConcurrentTimeSet final {
    struct Empty {};
    //hash map is being used as set, second template argument (Empty) is unused filler.
    typedef tbb::concurrent_hash_map<Frame *, Empty, StdHashCompare<Frame *>> SetType;
public:
    ConcurrentTimeSet() = default;
    ConcurrentTimeSet(ConcurrentTimeSet const &o) = default;
    ConcurrentTimeSet &operator=(ConcurrentTimeSet const &o) = default;
    ConcurrentTimeSet(ConcurrentTimeSet &&o) noexcept :
        set()
    {
        set.swap(o.set);
    }
    ConcurrentTimeSet &operator=(ConcurrentTimeSet &&o) noexcept
    {
        set.swap(o.set);
        return *this;
    }
    //Must never try to add or remove a particular frame concurrently,
    //only has safe concurrent access when called with different frames
    void add(Frame *toAdd);
    void remove(Frame *toRemove);
    void swap(ConcurrentTimeSet &o) {
        set.swap(o.set);
    }
    void clear() {
        set.clear();
    }
    bool empty() const {
        return set.empty();
    }
    std::size_t size() const {
        return set.size();
    }
    typedef boost::range_iterator<boost::select_first_range<SetType>>::type iterator;
    typedef boost::range_iterator<boost::select_first_range<SetType> const>::type const_iterator;
    typedef boost::range_reference<boost::select_first_range<SetType>>::type reference;
    typedef boost::range_reference<boost::select_first_range<SetType> const>::type const_reference;
    typedef boost::range_pointer<boost::select_first_range<SetType>>::type pointer;
    typedef boost::range_pointer<boost::select_first_range<SetType> const>::type const_pointer;
    iterator begin() {
        return boost::begin(boost::adaptors::keys(set));
    }
    const_iterator begin() const {
        return boost::begin(boost::adaptors::keys(set));
    }
    iterator end() {
        return boost::end(boost::adaptors::keys(set));
    }
    const_iterator end() const {
        return boost::end(boost::adaptors::keys(set));
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
    SetType set;
};
}
#endif //HG_CONCURRENT_TIME_SET_H
