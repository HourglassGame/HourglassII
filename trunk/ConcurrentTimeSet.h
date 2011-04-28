#ifndef HG_CONCURRENT_TIME_SET_H
#define HG_CONCURRENT_TIME_SET_H
#include <tbb/concurrent_hash_map.h>
#include <boost/range/adaptor/map.hpp>
#include <boost/range.hpp>
#include "BoostHashCompare.h"
#include "Frame.h"
namespace hg {
    template<typename Iterator>
    class MapToSetIteratorAdaptor
    {
        public:
        typedef typename Iterator::value_type::first_type value_type;
        typedef typename Iterator::difference_type  difference_type;
        typedef typename Iterator::value_type::first_type* pointer;
        typedef typename Iterator::value_type::first_type& reference;
        typedef typename Iterator::iterator_category  iterator_category;
        
        MapToSetIteratorAdaptor() :
        iterator_()
        {
        }
        MapToSetIteratorAdaptor(const Iterator& iterator) :
        iterator_(iterator)
        {
        }
        MapToSetIteratorAdaptor(const MapToSetIteratorAdaptor& other) :
        iterator_(other.iterator_)
        {
        }
        bool operator==(const MapToSetIteratorAdaptor& other)
        {
            return iterator_ == other.iterator_;
        }
        bool operator!=(const MapToSetIteratorAdaptor& other)
        {
            return !(*this==other);
        }
        typename MapToSetIteratorAdaptor::reference operator*() const
        {
            return (*iterator_).first;
        }
        typename MapToSetIteratorAdaptor::pointer operator->() const
        {
            return &iterator_->first;
        }
        MapToSetIteratorAdaptor& operator++()
        {
            ++iterator_;
            return *this;
        }
        MapToSetIteratorAdaptor  operator++(int)
        {
            MapToSetIteratorAdaptor retv(*this);
            ++(*this);
            return retv;
        }
        MapToSetIteratorAdaptor& operator--()
        {
            --iterator_;
            return *this;
        }
        MapToSetIteratorAdaptor  operator--(int)
        {
            MapToSetIteratorAdaptor retv(*this);
            --(*this);
            return retv;
        }
        private:
        Iterator iterator_;
    };
    
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
        struct Empty{};
        //hash map is being used as set, second template argument (Empty) is unused filler.
        typedef tbb::concurrent_hash_map<Frame*, Empty, BoostHashCompare<Frame*> > SetType;
        public:
        ConcurrentTimeSet();
        //Must never try to add or remove a particular frame concurrently,
        //only has safe concurrent access when called with different frames
        void add(Frame* toAdd);
        void remove(Frame* toRemove);
        void clear() { set_.clear(); }
        bool empty() const { return set_.empty(); }
        size_t size() const { return set_.size(); }
        typedef boost::range_iterator<boost::select_first_range<SetType> >::type iterator;
        typedef boost::range_iterator<const boost::select_first_range<SetType> >::type const_iterator;
        iterator begin() { return boost::begin(boost::adaptors::keys(set_)); }
        const_iterator begin() const { return boost::begin(boost::adaptors::keys(set_)); }
        iterator end() { return boost::end(boost::adaptors::keys(set_)); }
        const_iterator end() const { return boost::end(boost::adaptors::keys(set_)); }
        private:
        SetType set_;
    };
}
#endif //HG_CONCURRENT_TIME_SET_H
