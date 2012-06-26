#ifndef HG_TBB_ALLOCATOR_H
#define HG_TBB_ALLOCATOR_H
#include <tbb/scalable_allocator.h>
#include <boost/move/move.hpp>
#include <utility>
namespace hg {
//Adapts tbb::scalable_allocator to meet C++11 allocator requirements
template<typename T>
class tbb_scalable_allocator
{
private:
    template<typename P> struct Alloc
        { typedef tbb::scalable_allocator<P> type; };
    typedef typename Alloc<T>::type tbb_alloc;
    tbb_alloc alloc;
public:
    typedef typename tbb_alloc::pointer          pointer;
    typedef typename tbb_alloc::const_pointer    const_pointer;
    typedef void*       void_pointer;
    typedef void const* const_void_pointer;
    typedef typename tbb_alloc::value_type value_type;
    typedef typename tbb_alloc::size_type size_type;
    typedef typename tbb_alloc::difference_type difference_type;
    template<typename U> struct rebind
        { typedef tbb_scalable_allocator<U> other; };
    
    typedef typename tbb_alloc::reference reference;
    typedef typename tbb_alloc::const_reference const_reference;
    
    pointer allocate(size_type n, void const* u = 0) { return alloc.allocate(n, u); }
    void deallocate(pointer p, size_type n) { alloc.deallocate(p, n); }
    size_type max_size() const { return alloc.max_size(); }
    
    tbb_scalable_allocator() : alloc() {}
    tbb_scalable_allocator(tbb_scalable_allocator const&)
        : alloc() {}
    template<typename U>
    tbb_scalable_allocator(tbb_scalable_allocator<U> const&)
        : alloc() {}
    
    template<typename C, typename Args>
    void construct(C* c, BOOST_FWD_REF(Args) args) {
        ::new(static_cast<void*>(c)) C(boost::forward<Args>(args));
    }
    template<typename C>
    void destroy(C* c) { c->~C(); }
    
    tbb_scalable_allocator<T> select_on_container_copy_construction() const { return *this; }
};

template<typename T, typename U>
inline bool operator==(
    tbb_scalable_allocator<T> const&,
    tbb_scalable_allocator<U> const&) { return true; }

template<typename T, typename U>
inline bool operator!=(
    tbb_scalable_allocator<T> const&,
    tbb_scalable_allocator<U> const&) { return false; }
}//namespace hg
#endif //HG_TBB_ALLOCATOR_H
