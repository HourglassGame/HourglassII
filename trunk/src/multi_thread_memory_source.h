#ifndef HG_MULTI_THREAD_MEMORY_SOURCE_H
#define HG_MULTI_THREAD_MEMORY_SOURCE_H
#include "multi_thread_allocator.h"
namespace hg {
    struct multi_thread_memory_source {
        void *alloc(std::size_t size) const { return multi_thread_operator_new(size); }
        void free(void *toFree) const { multi_thread_operator_delete(toFree); }
    };
}
#endif //HG_MULTI_THREAD_MEMORY_SOURCE_H
