#ifndef HG_TBB_ALLOCATOR_H
#define HG_TBB_ALLOCATOR_H
#include <tbb/scalable_allocator.h>
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
    typedef void       *void_pointer;
    typedef void const *const_void_pointer;
    typedef typename tbb_alloc::value_type value_type;
    typedef typename tbb_alloc::size_type size_type;
    typedef typename tbb_alloc::difference_type difference_type;
    template<typename U> struct rebind final
        { typedef tbb_scalable_allocator<U> other; };

    typedef typename tbb_alloc::reference reference;
    typedef typename tbb_alloc::const_reference const_reference;

    pointer allocate(size_type n, void const *u = nullptr) { return alloc.allocate(n, u); }
    void deallocate(pointer p, size_type n) { alloc.deallocate(p, n); }
    size_type max_size() const { return alloc.max_size(); }

    tbb_scalable_allocator() : alloc() {}
    tbb_scalable_allocator(tbb_scalable_allocator const &)
        : alloc() {}
    template<typename U>
    tbb_scalable_allocator(tbb_scalable_allocator<U> const &)
        : alloc() {}

    template<typename C, typename ...Args>
    void construct(C *c, Args &&...args) {
        ::new(const_cast<void*>(static_cast<void const*>(c))) C(std::forward<Args>(args)...);
    }
    template<typename C>
    void destroy(C *c) { c->~C(); }

    tbb_scalable_allocator<T> select_on_container_copy_construction() const { return *this; }
};

template<>
class tbb_scalable_allocator<void>
{
private:
    template<typename P> struct Alloc
        { typedef tbb::scalable_allocator<P> type; };
    typedef Alloc<void>::type tbb_alloc;
    tbb_alloc alloc;
public:
    typedef tbb_alloc::pointer          pointer;
    typedef tbb_alloc::const_pointer    const_pointer;
    typedef void       *void_pointer;
    typedef void const *const_void_pointer;
    typedef tbb_alloc::value_type value_type;

    template<typename U> struct rebind final
        { typedef tbb_scalable_allocator<U> other; };

    tbb_scalable_allocator() : alloc() {}
    tbb_scalable_allocator(tbb_scalable_allocator const &)
        : alloc() {}
    template<typename U>
    tbb_scalable_allocator(tbb_scalable_allocator<U> const &)
        : alloc() {}

    tbb_scalable_allocator<void> select_on_container_copy_construction() const { return *this; }
};

template<typename T, typename U>
inline bool operator==(
    tbb_scalable_allocator<T> const &,
    tbb_scalable_allocator<U> const &) { return true; }

template<typename T, typename U>
inline bool operator!=(
    tbb_scalable_allocator<T> const &,
    tbb_scalable_allocator<U> const &) { return false; }
}//namespace hg
#endif //HG_TBB_ALLOCATOR_H
