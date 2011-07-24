#ifndef HG_MULTI_THREAD_ALLOCATOR_H
#define HG_MULTI_THREAD_ALLOCATOR_H
#include "ned_allocator.h"
namespace hg {
    template<typename T> struct multi_thread_allocator {
        //The allocator type to use in cases where there
        //are many threads simultaneously wanting
        //to perform allocation/deallocation.
        typedef nedalloc::ned_allocator<T> type;
    };
}
#endif //HG_MULTI_THREAD_ALLOCATOR_H
