#ifndef HG_MULTI_THREAD_ALLOCATOR_H
#define HG_MULTI_THREAD_ALLOCATOR_H
//The purpose of this file is to provide a single place that needs to be changed
//to modify the memory allocation used in the regions of the code that execute concurrently.

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>

#include <tbb/scalable_allocator.h>

//#include "scalable_allocator.h"
//#include "forward.h"
namespace hg {
    //Metafunction to calculate the allocator type to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    template<typename T> struct multi_thread_allocator {
        typedef tbb::scalable_allocator<T> type;
    };

    //Versions of the C library functions to use
    //when multiple threads may be performing allocations/deallocations
    //simultaneously.
    inline void* multi_thread_malloc(std::size_t size) { return scalable_malloc(size); }
    inline void  multi_thread_free(void* p) { return scalable_free(p); }
    inline void* multi_thread_calloc(size_t n, size_t size) { return scalable_calloc(n, size); }
    inline void* multi_thread_realloc(void* p, size_t size) { return scalable_realloc(p, size); }
    
    inline void* multi_thread_operator_new(std::size_t size) {
        while (true) {
            if (void* pointer = multi_thread_malloc(size)) {
                return pointer;
            }
            if (std::new_handler handler = std::set_new_handler(0)) {
                std::set_new_handler(handler);
                (*handler)();
            }
            else {
                throw std::bad_alloc();
            }
        }
    }
    inline void multi_thread_operator_delete(void* p) {
        multi_thread_free(p);
    }
    //multi_thread_new<T>(args...)
    //is equivalent to:
    //  new T(args...),
    //except that it performs its allocation
    //using `multi_thread_operator_new`.
    
    //C++11 version
    /*
    template<typename T, typename... Args>
    T* multi_thread_new(Args&&...args){
    	void* p(multi_thread_operator_new(sizeof(T)));
    	try {
    		return new (p) T(hg::forward<Args>(args)...);
    	}
    	catch (...) {
    		multi_thread_operator_delete(p);
    		throw;
    	}
    }*/

    //C++03 version:
    //Forwarding is emulated with "by value" passing.
    //Use boost::ref or boost::cref to get reference behaviour.
    
#ifndef HG_MAX_MULTI_THREAD_NEW_PARAMS
# define HG_MAX_MULTI_THREAD_NEW_PARAMS 3
#endif

#define HG_MULTI_THREAD_NEW(Z, N, _)                                                \
    template<typename T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, typename A)>   \
        T* multi_thread_new(BOOST_PP_ENUM_BINARY_PARAMS(N, A, a)) {                 \
        void* p(multi_thread_operator_new(sizeof(T)));                              \
        try {                                                                       \
            return new (p) T(BOOST_PP_ENUM_PARAMS(N, a));                           \
        }                                                                           \
        catch (...) {                                                               \
            multi_thread_operator_delete(p);                                        \
            throw;                                                                  \
        }                                                                           \
    }

BOOST_PP_REPEAT(BOOST_PP_INC(HG_MAX_MULTI_THREAD_NEW_PARAMS), HG_MULTI_THREAD_NEW, _)

#undef HG_MULTI_THREAD_NEW

}
#endif //HG_MULTI_THREAD_ALLOCATOR_H
