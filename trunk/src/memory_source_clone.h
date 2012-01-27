#ifndef HG_MEMORY_SOURCE_CLONE_H
#define HG_MEMORY_SOURCE_CLONE_H
namespace hg {
    //MemorySource is kind of like an Allocator, except that its functionality
    //is closer to malloc and free.
    template<typename T, typename MemorySource>
    struct memory_source_clone :private MemorySource {
        T* new_clone(T const& toClone) const {
            void* p(MemorySource::alloc(toClone.clone_size()));
            try {
                return toClone.perform_clone(p);
            } catch (...) {
                MemorySource::free(p);
                throw;
            }
        }
        void delete_clone(T* toDelete) const {
            if (toDelete) {
                toDelete->~T();
                MemorySource::free(toDelete);
            }
        }
    };
}
#endif //HG_MEMORY_SOURCE_CLONE_H
