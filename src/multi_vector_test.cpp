#include "multi_vector.h"
#include "TestDriver.h"
#include <set>
namespace hg {
namespace multi_vector_test {
namespace {
    struct ObjectLog final
    {
        std::set<void const*> live;
        std::multiset<void const*> multi_constructed;
        std::multiset<void const*> multi_deleted;

        std::set<void const*> invalid_constructed;
        std::set<void const*> invalid_deleted;

        std::set<std::pair<void const*, void const*>> live_regions;
        std::set<std::pair<void const*, void const*>> multi_constructed_regions;
        std::set<std::pair<void const*, void const*>> multi_deleted_regions;

        bool is_valid() const {
            return multi_constructed.empty() && multi_deleted.empty()
                && invalid_constructed.empty() && invalid_deleted.empty()
                && multi_constructed_regions.empty() && multi_deleted_regions.empty()
                ;
        }

        void LogAllocation(std::pair<void const*, void const*> const &r) {
            auto const it{live_regions.find(r)};
            if (it == live_regions.end()) {
                live_regions.insert(r);
            }
            else {
                multi_constructed_regions.insert(r);
            }
        }
        void LogDeallocation(std::pair<void const*, void const*> const &r) {
            //TODO: Check that r contains no live objects
            auto const it{live_regions.find(r)};
            if (it != live_regions.end()) {
                live_regions.erase(it);
            }
            else {
                multi_deleted_regions.insert(r);
            }
        }

        void LogConstruction(void const *t) {
            if (boost::find_if(live_regions, [t](auto const &r) {return r.first <= t && t < r.second;}) == live_regions.end()) {
                invalid_constructed.insert(t);
            }
            auto const it{live.find(t)};
            if (it == live.end()) {
                live.insert(t);
            }
            else {
                multi_constructed.insert(t);
            }
        }
        void LogDestruction(void const *t) {
            if (boost::find_if(live_regions, [t](auto const &r) {return r.first <= t && t < r.second; }) == live_regions.end()) {
                invalid_deleted.insert(t);
            }
            auto const it{live.find(t)};
            if (it != live.end()) {
                live.erase(it);
            }
            else {
                multi_deleted.insert(t);
            }
        }
    };
    template<typename T>
    struct LoggingAllocator {
    private:
        void attemptOperation() {
            if (*operationsToFail != 0) {
                --(*operationsToFail);
                if (*operationsToFail == 0) {
                    throw std::exception();
                }
            }
        }
    public:
        ObjectLog * l;
        std::size_t *operationsToFail;
        LoggingAllocator(ObjectLog &l, std::size_t &operationsToFail) :
            l(&l), operationsToFail(&operationsToFail)
        {}
        template<typename U>
        LoggingAllocator(LoggingAllocator<U> const &o) :
            l(o.l), operationsToFail(o.operationsToFail)
        {}

        typedef T value_type;

        T *allocate(std::size_t n) {
            attemptOperation();
            //TODO: check for integer overflow!!
            auto const p{static_cast<T*>(::operator new(n * sizeof(T)))};
            l->LogAllocation(std::make_pair(static_cast<void*>(p), static_cast<void*>(p+n)));
            return p;
        }
        void deallocate(T *p, std::size_t n) {
            ::operator delete(p);
            l->LogDeallocation(std::make_pair(static_cast<void*>(p), static_cast<void*>(p+n)));
        }
        template<typename... Args>
        void construct(void *p, Args&&... args) {
            attemptOperation();
            //TODO: Check *p is within a live memory region!!
            new (p) T(std::forward<Args>(args)...);
            l->LogConstruction(p); //TODO: bad result occurs if this throws! Need to ensure test environment has sufficient resources
        }
        void destroy(T const *p) {
            //TODO: Check *p is within a live memory region!!
            p->~T();
            l->LogDestruction(p); //TODO: bad result occurs if this throws! Need to ensure test environment has sufficient resources
        }

        bool operator==(LoggingAllocator const &o) const {return l == o.l && operationsToFail == o.operationsToFail;}
        bool operator!=(LoggingAllocator const &o) const {return !(*this == o);}
        typedef std::false_type is_always_equal;
    };

    template<typename T, std::size_t N_dims>
    bool checkInvariants(hg::multi_vector<T, N_dims, LoggingAllocator<T>> const &v) {
        //TODO: Check multiple v at once!
        auto const &alloc{v.get_allocator()};
        auto r{boost::adaptors::transform(v, [](T const &t) {return static_cast<void const*>(&t); })};
        std::set<void const*> vObj(boost::begin(r), boost::end(r));
        return
            //No Double Construction or Destruction.
            alloc.l->is_valid()
            //Live objects exactly match contents of vectors
         && alloc.l->live == vObj
         && vObj.size() == v.total_size_by_dim(0)
            //Live memory regions exactly match capacity() and data() of vectors
            //TODO
            //Size fits within capacity of vector
         && regionFitsInRegion(v.size(), v.capacity())
            ;
    }

    bool checkObjectLogClean(ObjectLog const &l) {
        return l.live.empty() && l.is_valid();
    }

    template<typename F>
    auto runTest(F &&do_test) {
        return [&] {
            bool lastTestThrew{true};
            for (std::size_t i{1}; lastTestThrew; ++i) {
                std::size_t timeToFail{i};
                ObjectLog l;
                LoggingAllocator<void> alloc(l, timeToFail);
                try {
                    if (!do_test(alloc)) {
                        return false;
                    }
                    lastTestThrew = false;
                }
                catch(...){
                    if (!checkObjectLogClean(l)) {
                        return false;
                    }
                    lastTestThrew = true;
                }
            }
            return true;
        };
    }


    bool testDefaultCtor(LoggingAllocator<void> &alloc)
    {
        typedef hg::multi_vector<int, 100, LoggingAllocator<int>> arr;
        arr t{ std::allocator_traits<LoggingAllocator<void>>::rebind_alloc<int>(alloc) };
        return
               checkInvariants(t)
            && t.size()     == std::array<arr::index, 100>{}
            && t.capacity() == std::array<arr::index, 100>{};
    }

    bool testRangeCtor(LoggingAllocator<void> &alloc)
    {
        typedef hg::multi_vector<int, 3, LoggingAllocator<int>> arr;
        std::array<std::size_t, 3> size{2,1,3};
        std::array<int, 6> data{4,5,123,-124,60000,9};
        arr t{ size, data, std::allocator_traits<LoggingAllocator<void>>::rebind_alloc<int>(alloc) };
        return
            checkInvariants(t)
            && t.size() == size
            && boost::equal(data, t);
    }

    bool testResize(LoggingAllocator<void> &alloc) {
        typedef hg::multi_vector<int, 3, LoggingAllocator<int>> arr;
        std::array<std::size_t, 3> size1{2,1,3};
        std::array<int, 6> data1{4,5,123,-124,60000,9};
        arr t{ size1, data1, std::allocator_traits<LoggingAllocator<void>>::rebind_alloc<int>(alloc) };
        if (!(
               checkInvariants(t)
            && t.size() == size1
            && boost::equal(data1, t))) return false;

        std::array<std::size_t, 3> size2{2,1,2};
        std::array<int, 4> data2{4,5,-124,60000};
        t.resize(size2);
        if (!(
            checkInvariants(t)
            && t.size() == size2
            && boost::equal(data2, t))) return false;

        std::array<std::size_t, 3> size3{2,2,2};
        std::array<int, 8> data3{4,5,0,0,-124,60000,0,0};
        t.resize(size3);
        if (!(
            checkInvariants(t)
            && t.size() == size3
            && boost::equal(data3, t))) return false;

        return true;
    }
    struct tester final {
        tester() {
            ::hg::getTestDriver().registerUnitTest("multi_vector_testDefaultCtor", runTest(testDefaultCtor));
            ::hg::getTestDriver().registerUnitTest("multi_vector_testRangeCtor", runTest(testRangeCtor));
            ::hg::getTestDriver().registerUnitTest("multi_vector_testResize", runTest(testResize));
        }
    } tester;
}
}
}
