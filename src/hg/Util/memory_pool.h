#ifndef HG_MEMORY_POOL_H
#define HG_MEMORY_POOL_H
#include "hg/mt/std/vector"
#include <memory>
#include <cassert>

#include <tbb/scalable_allocator.h>
namespace hg {
    struct user_allocator_tbb_alloc final {
        typedef std::size_t  size_type;
        typedef std::ptrdiff_t difference_type;

        static char *malloc(const size_type sz) {
            auto const p{static_cast<char *>(scalable_malloc(sz))};
            if (!p) throw std::bad_alloc();
            return p;
        }

        static std::size_t msize(void *p) noexcept {
            return scalable_msize(p);
        }

        static void free(char *const p) noexcept {
            scalable_free(p);
        }
    };
    template<typename UserAllocator>
    struct memory_region final {
        explicit memory_region(std::size_t size) :
            current(UserAllocator::malloc(size)), max_space(scalable_msize(current)), space(max_space)
        {
            assert(max_space >= size);
#if 0
            if (size != max_space) {
                std::cout << "requested: " << size << " actual: " << max_space << "\n" << std::flush;
                __debugbreak();
            }
#endif
        }
        memory_region(memory_region const&) = delete;
        memory_region(memory_region &&o) noexcept :
            current(o.current), max_space(o.max_space), space(o.space)
        {
            o.current = nullptr;
            o.max_space = 0;
            o.space = 0;
        }
        memory_region &operator=(memory_region const&) = delete;
        memory_region &operator=(memory_region &&o) noexcept {
            boost::swap(current, o.current);
            boost::swap(max_space, o.max_space);
            boost::swap(space, o.space);
            return *this;
        }
        ~memory_region() noexcept {
            if (current) {
                reset();
                UserAllocator::free(static_cast<char*>(current));
            }
        }
        void *try_malloc(std::size_t const size, std::size_t const align = alignof (std::max_align_t)) noexcept {
            //TODO: assert align is power of 2??
            if (!current) { return nullptr; }
            auto const nextAligned{std::align(align, size, current, space)};
            if (nextAligned) {
                current = static_cast<char*>(current)+size;
                space -= size;
            }
            return nextAligned;
        }
        void reset() noexcept {
            current = static_cast<char*>(current)-(max_space-space);
            space = max_space;
        }
        std::size_t size() const noexcept {
            return max_space;
        }
    private:
        void *current;
        std::size_t max_space;
        std::size_t space;
    };

#define USE_POOL_ALLOCATOR 1
#if USE_POOL_ALLOCATOR
    //Rename this to bump_allocator??

    //Memory allocator that is
    //efficient because it can assume
    //that it is only ever called from a single thread,
    //and deallocation can happen in bulk.
    template<typename UserAllocator = user_allocator_tbb_alloc>
    struct memory_pool final {
    private:
        //std::size_t largestRegion;
        mt::std::vector<memory_region<UserAllocator>> regions;
        //TODO: does this 'back_region' cache actually effect performance at all?
        memory_region<UserAllocator> *back_region;
    public:
        explicit memory_pool(std::size_t initialSize = 1 << 13):
            /*largestRegion(initialSize), */regions(), back_region()
        {
            regions.emplace_back(initialSize);
            back_region = &regions.back();
        }
        memory_pool(memory_pool &&o) noexcept = default;

        void *malloc(std::size_t const size, std::size_t const align = alignof(std::max_align_t)) {
            assert(!regions.empty());
            assert(back_region == &regions.back());
            auto const p{back_region->try_malloc(size, align)};
            if (p) return p;
#if 0
            for (auto &&r : regions) {
                auto const p{r.try_malloc(size, align)};
                if (p) return p;
            }
#endif
            auto const largestRegion = std::max(back_region->size()*2, size);
            regions.emplace_back(largestRegion);
            back_region = &regions.back();
            //try_malloc is guaranteed to succeed if 'regions.emplace_back(largestRegion);' didn't throw.
            return back_region->try_malloc(size, align);
        }
        void consolidate_memory() {
            assert(!regions.empty());
            assert(back_region == &regions.back());
            if (regions.size() == 1) {
                back_region->reset();
            }
            else {
                //__debugbreak();
                auto const largestRegion = back_region->size();
                regions.clear();
                regions.emplace_back(largestRegion*2);
                back_region = &regions.back();
            }
            assert(back_region == &regions.back());
        }
    };
#else
    template<typename UserAllocator = user_allocator_tbb_alloc>
    struct memory_pool final {
    public:
        explicit memory_pool(std::size_t initialSize = 1 << 13)
        {
        }
        memory_pool(memory_pool &&o) noexcept = delete;

        void *malloc(std::size_t const size, std::size_t const align = alignof(std::max_align_t)) {
            assert(false);
            return nullptr;
        }
        void consolidate_memory() {
        }
    };
#endif
#if USE_POOL_ALLOCATOR
    template<typename T>
    struct memory_pool_allocator {
    public:
        memory_pool<user_allocator_tbb_alloc> *pool;

        typedef T value_type;
        typedef std::true_type is_always_equal;

        memory_pool_allocator(memory_pool_allocator const &o) noexcept :
            pool(o.pool)
        {
        }

        template<typename U>
        memory_pool_allocator(memory_pool_allocator<U> const &o) noexcept :
            pool(o.pool)
        {
        }

        memory_pool_allocator &operator=(memory_pool_allocator const&) noexcept = default;
        ~memory_pool_allocator() noexcept = default;

        memory_pool_allocator(memory_pool<user_allocator_tbb_alloc> &pool) : pool(&pool) {}

        T *allocate(std::size_t n) {
            return static_cast<T*>(pool->malloc(n * sizeof(T), alignof (T)));
        }

        void deallocate(T *, std::size_t) noexcept {
            //Do nothing
        }
        bool operator==(memory_pool_allocator const&) const {return true;}
        bool operator!=(memory_pool_allocator const&) const {return false;}
    };
#else
    template<typename T>
    struct memory_pool_allocator : tbb_scalable_allocator<T> {
    public:

        typedef T value_type;
        typedef std::true_type is_always_equal;

        memory_pool_allocator(memory_pool_allocator const &)
        {
        }

        template<typename U>
        memory_pool_allocator(memory_pool_allocator<U> const &)
        {
        }

        memory_pool_allocator &operator=(memory_pool_allocator const&) = default;
        ~memory_pool_allocator() = default;

        memory_pool_allocator(memory_pool<user_allocator_tbb_alloc> &)
        {}

        bool operator==(memory_pool_allocator const&) const { return true; }
        bool operator!=(memory_pool_allocator const&) const { return false; }
    };
#endif

    
#if USE_POOL_ALLOCATOR
    struct memory_pool_memory_source {
    private:
        memory_pool<user_allocator_tbb_alloc> *pool;
    public:
        memory_pool_memory_source(memory_pool<user_allocator_tbb_alloc> &pool) : pool(&pool) {}
        void *alloc(std::size_t const size, std::size_t const align = alignof(std::max_align_t)) const {
            return pool->malloc(size, align);
        }
        void free(void *const) const {
            //Do Nothing.
        }
    };
#else
    struct memory_pool_memory_source : multi_thread_memory_source {
    public:
        memory_pool_memory_source(memory_pool<user_allocator_tbb_alloc> &) {}
    };
#endif
}
#if USE_POOL_ALLOCATOR
inline void* operator new(std::size_t count, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) {
    //TODO: Support new_handler?
    return pool.malloc(count);
}

inline void operator delete(void *p, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) noexcept {
    //Do Nothing
    (void)p; (void)pool;
}
#else
inline void* operator new(std::size_t count, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) {
    return hg::multi_thread_operator_new(count);
}

inline void operator delete(void *p, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) noexcept {
    hg::multi_thread_operator_delete(p);
}
#endif
#if USE_POOL_ALLOCATOR
    template <typename T>
    struct memory_pool_deleter final {
        void operator()(T *const p) const {
            if (p) {
                p->~T();
            }
        }
    };
    template <typename T>
    struct memory_pool_deleter<T[]>;
#else
    template <typename T>
    struct memory_pool_deleter final {
        void operator()(T *ptr) const { multi_thread_delete(ptr); }
    };
    template <typename T>
    struct memory_pool_deleter<T[]>;
#endif

#endif
