#ifndef HG_MULTI_THREAD_ALLOCATOR_H
#define HG_MULTI_THREAD_ALLOCATOR_H
//The purpose of this file is to provide a single place that needs to be changed
//to modify the memory allocation used in the regions of the code that execute concurrently.

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>

#include <tbb/scalable_allocator.h>
#include "scalable_allocator.h"

#include "forward.h"

#include <boost/config.hpp>

#include <new>
//#include "scalable_allocator.h"
//#include "forward.h"
namespace hg {
    //Metafunction to calculate the allocator type to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    template<typename T> struct multi_thread_allocator final {
        typedef tbb_scalable_allocator<T> type;
    };
    
    template<typename T>
    using multi_thread_allocator_t = tbb_scalable_allocator<T>;

    //Versions of the C library functions to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    inline void *multi_thread_malloc(std::size_t size) { return scalable_malloc(size); }
    inline void  multi_thread_free(void *p) { return scalable_free(p); }
    inline void *multi_thread_calloc(size_t n, size_t size) { return scalable_calloc(n, size); }
    inline void *multi_thread_realloc(void *p, size_t size) { return scalable_realloc(p, size); }
    
    inline void *multi_thread_operator_new(std::size_t size) {
        while (true) {
            if (void *pointer = multi_thread_malloc(size)) {
                return pointer;
            }
            if (std::new_handler handler = std::set_new_handler(nullptr)) {
                std::set_new_handler(handler);
                (*handler)();
            }
            else {
                throw std::bad_alloc();
            }
        }
    }
    inline void multi_thread_operator_delete(void *p) {
        multi_thread_free(p);
    }
    
    template<typename T>
    void multi_thread_delete(T *p) {
        if (p) {
            p->~T();
            multi_thread_operator_delete(p);
        }
    }

    struct multi_thread_tag{};

}
//new (multi_thread_tag{}) T(args...)
    //is equivalent to:
    //  new T(args...),
    //except that it performs its allocation
    //using `multi_thread_operator_new`.
inline void* operator new(std::size_t count, hg::multi_thread_tag) {
    return hg::multi_thread_operator_new(count);
}

inline void operator delete(void *p, hg::multi_thread_tag) noexcept {
    hg::multi_thread_operator_delete(p);
}

#endif //HG_MULTI_THREAD_ALLOCATOR_H
