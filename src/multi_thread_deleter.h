#ifndef HG_MULTI_THREAD_DELETER_H
#define HG_MULTI_THREAD_DELETER_H
#include "multi_thread_allocator.h"
namespace hg {
    template <typename T>
    struct multi_thread_deleter {
        void operator()(T* ptr) const { multi_thread_delete(ptr); }
    };
    template <typename T>
    struct multi_thread_deleter<T[]>;
}
#endif //HG_MULTI_THREAD_DELETER_H
