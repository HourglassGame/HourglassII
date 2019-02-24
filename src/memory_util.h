#ifndef HG_MEMORY_UTIL_H
#define HG_MEMORY_UTIL_H
#include <type_traits>
namespace hg {
    //the enable_if<is_trivialy_copyable> isn't
    //required for this function to be correct; but in the current usecases,
    //the storage for the objects is reused by this function, without
    //any destructors being expicitly called, so we want to detect if there are ever
    //non-trivial destructors that would make this usage incorrect.
    template<typename InputIt, typename = std::enable_if_t<std::is_trivially_copyable_v<typename std::iterator_traits<InputIt>::value_type>>>
    void copy_pod_to_storage(
        InputIt first,
        InputIt const last,
        void *startOfStorage
    )
    {
        //Like std::uninitialized_copy, but doesn't
        //demand pointer-arithmatic on its uninitialized
        //output parameter; and so avoids undefined behaviour.
        //See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0593r3.html#dynamic-construction-of-arrays

        for (; first != last; ++first) {
            new (startOfStorage) typename std::iterator_traits<InputIt>::value_type{ *first };
            startOfStorage = static_cast<char *>(startOfStorage)+sizeof(typename std::iterator_traits<InputIt>::value_type);
        }
    }
}
#endif