#ifndef HG_CONCURRENT_TIME_SET_H
#define HG_CONCURRENT_TIME_SET_H
#include <tbb/concurrent_hash_map.h>
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
        typedef MapToSetIteratorAdaptor<SetType::iterator> iterator;
        typedef MapToSetIteratorAdaptor<SetType::const_iterator> const_iterator;
        iterator begin() { return set_.begin(); }
        const_iterator begin() const { return set_.begin(); }
        iterator end() { return set_.end(); }
        const_iterator end() const { return set_.end(); }
        private:
        SetType set_;
    };
}
#endif //HG_CONCURRENT_TIME_SET_H
