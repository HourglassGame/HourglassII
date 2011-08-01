#ifndef HG_MULTI_THREAD_ALLOCATOR_H
#define HG_MULTI_THREAD_ALLOCATOR_H
//The purpose of this file is to provide a single place that needs to be changed
//to modify the memory allocation used in the regions of the code that execute concurrently.

#include <tbb/scalable_allocator.h>
namespace hg {
    //Metafunction to calculate the allocator type to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    template<typename T> struct multi_thread_allocator {
        typedef tbb::scalable_allocator<T> type;
    };

    //Versions of the C library functions to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    inline void* multi_thread_malloc(std::size_t size) { return scalable_malloc(size); }
    inline void  multi_thread_free(void* p) { return scalable_free(p); }
    inline void* multi_thread_calloc(size_t n, size_t size) { return scalable_calloc(n, size); }
    inline void* multi_thread_realloc(void* p, size_t size) { return scalable_realloc(p, size); }
    
    inline void* multi_thread_operator_new(std::size_t size) {
        while (true) {
            if (void* pointer = multi_thread_malloc(size)) {
                return pointer;
            }
            if (std::new_handler handler = std::set_new_handler(0)) {
                std::set_new_handler(handler);
                (*handler)();
            }
            else {
                throw std::bad_alloc();
            }
        }
    }
    inline void multi_thread_operator_delete(void* p) {
        multi_thread_free(p);
    }
}
#endif //HG_MULTI_THREAD_ALLOCATOR_H
