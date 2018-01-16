#ifndef HG_MEMORY_POOL_H
#define HG_MEMORY_POOL_H
#include "mt/std/vector"
#include <memory>
#include <cassert>

#include "tbb/scalable_allocator.h"
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
    //Memory allocator that is
    //efficient because it can assume
    //that it is only ever called from a single thread,
    //and deallocation can happen in bulk.
    template<typename UserAllocator = user_allocator_tbb_alloc>
    struct memory_pool final {
    private:
        //std::size_t largestRegion;
        mt::std::vector<memory_region<UserAllocator>> regions;
    public:
        explicit memory_pool(std::size_t initialSize = 1 << 13):
            /*largestRegion(initialSize), */regions()
        {
            regions.emplace_back(initialSize);
        }
        memory_pool(memory_pool &&o) noexcept = delete;

        void *malloc(std::size_t const size, std::size_t const align = alignof(std::max_align_t)) {
            assert(!regions.empty());
            auto const p{regions.back().try_malloc(size, align)};
            if (p) return p;
#if 0
            for (auto &&r : regions) {
                auto const p{r.try_malloc(size, align)};
                if (p) return p;
            }
#endif
            auto const largestRegion = std::max(regions.back().size()*2, size);
            regions.emplace_back(largestRegion);
            return regions.back().try_malloc(size, align);
        }
        void consolidate_memory() {
            if (regions.size() == 1) {
                regions.back().reset();
            }
            else {
                //__debugbreak();
                auto const largestRegion = regions.back().size();
                regions.clear();
                regions.emplace_back(largestRegion*2);
            }
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
        //memory_pool<user_allocator_tbb_alloc> *pool;

        typedef T value_type;
        typedef std::true_type is_always_equal;

        memory_pool_allocator(memory_pool_allocator const &o)// :
        //    pool(o.pool)
        {
        }

        template<typename U>
        memory_pool_allocator(memory_pool_allocator<U> const &o)// :
        //    pool(o.pool)
        {
        }

        memory_pool_allocator &operator=(memory_pool_allocator const&) = default;
        ~memory_pool_allocator() = default;

        memory_pool_allocator(memory_pool<user_allocator_tbb_alloc> &pool) //: pool(&pool)
        {}

        bool operator==(memory_pool_allocator const&) const { return true; }
        bool operator!=(memory_pool_allocator const&) const { return false; }
    };
#endif
}
#endif
