#ifndef HG_DEFAULT_MEMORY_SOURCE_H
#define HG_DEFAULT_MEMORY_SOURCE_H
#include <cstddef>
namespace hg {
    struct default_memory_source final {
        void *alloc(std::size_t size) const { return operator new(size); }
        void free(void *toFree) const { operator delete(toFree); }
    };
}
#endif //HG_DEFAULT_MEMORY_SOURCE_H
