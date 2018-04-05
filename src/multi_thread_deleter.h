#ifndef HG_MULTI_THREAD_DELETER_H
#define HG_MULTI_THREAD_DELETER_H
#include "multi_thread_allocator.h"
namespace hg {
    template<typename T>
    struct multi_thread_deleter {
        constexpr multi_thread_deleter() noexcept = default;
        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
        multi_thread_deleter(multi_thread_deleter<U> const &) noexcept {}
        void operator()(T * const ptr) const {
            multi_thread_delete(ptr);
        }
    };
    template <typename T>
    struct multi_thread_deleter<T[]>;
}
#endif //HG_MULTI_THREAD_DELETER_H
