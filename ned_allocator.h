#ifndef NEDALLOC_NED_ALLOCATOR_H
#define NEDALLOC_NED_ALLOCATOR_H
#include "nedmalloc.h"
#include <limits>
#include <new>
namespace nedalloc {
template<typename T>
class ned_allocator;
template<>
class ned_allocator<void> {
public:
    typedef void* pointer;
    typedef void const* const_pointer;
    typedef void value_type;
    template<typename U> struct rebind {
        typedef ned_allocator<U> other;
    };
};
template<typename T>
class ned_allocator {
public:
    typedef T* pointer;
    typedef T const* const_pointer;
    typedef T& reference;
    typedef T const& const_reference;
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    template<typename U> struct rebind {
        typedef ned_allocator<U> other;
    };
    pointer address(reference r) const { return &r; }
    const_pointer address(const_reference s) const { return &s; }
    pointer allocate(size_type n) const {
        if (void* p = nedmalloc(n * sizeof(T))) {
            return static_cast<pointer>(p);
        }
        else {
            throw std::bad_alloc();
        }
    }
    template<typename U>
    pointer allocate(
            size_type n,
            typename rebind<U>::const_pointer u) const
    {
        return allocate(n);
    }
    void deallocate(pointer p, size_type n) const {
        nedfree(p);
    }
    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    ned_allocator(){}
    template<typename U> ned_allocator(ned_allocator<U>) { }
    void construct(pointer p, T const& t) {
        new(static_cast<void*>(p)) T(t);
    }
    void destroy(pointer p) {
        p->~T();
    }
};
template<typename T>
bool operator==(ned_allocator<T> const& a1, ned_allocator<T> const& a2) { return true; }
template<typename T>
bool operator!=(ned_allocator<T> const& a1, ned_allocator<T> const& a2) { return false; }
}//namespace nedalloc
#endif //NEDALLOC_NED_ALLOCATOR_H
