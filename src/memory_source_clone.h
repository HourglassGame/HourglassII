#ifndef HG_MEMORY_SOURCE_CLONE_H
#define HG_MEMORY_SOURCE_CLONE_H
namespace hg {
    //MemorySource is kind of like an Allocator, except that its functionality
    //is closer to malloc and free.
    template<typename T, typename MemorySource>
    struct memory_source_clone : private MemorySource {
        explicit memory_source_clone() : MemorySource() {}
        explicit memory_source_clone(MemorySource memorySource) : MemorySource(memorySource) {}


        T *new_clone(T const &toClone) const {
            //TODO: Handle custom alignment as well as custom size
            void *p(MemorySource::alloc(toClone.clone_size()));
            try {
                return toClone.perform_clone(p);
            } catch (...) {
                MemorySource::free(p);
                throw;
            }
        }
        void delete_clone(T *toDelete) const noexcept {
            if (toDelete) {
                toDelete->~T();
                MemorySource::free(toDelete);
            }
        }
    };
}
#endif //HG_MEMORY_SOURCE_CLONE_H
