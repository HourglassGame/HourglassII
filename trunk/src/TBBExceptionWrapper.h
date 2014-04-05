#ifndef HG_TBB_EXCEPTION_WRAPPER_H
#define HG_TBB_EXCEPTION_WRAPPER_H
#include <exception>
#include <tbb/tbb_exception.h>
#include <boost/exception_ptr.hpp>
#include <thread>
namespace hg {
    //TBBOuterExceptionWrapper and TBBInnerExceptionWrapper work together
    //to allow exceptions to (slightly more) properly be passed through the 
    //Threading Building Blocks scheduler (in C++03 mode).
    
    //TBBInnerExceptionWrapper wraps all known excepion types up in a boost::exception_ptr, and wraps that up
    //in a tbb::movable_exception.
    //TBBOuterExceptionWrapper catches the `movable_exception` and rethrows the exception inside the exception_ptr.
    //(TBBOuterExceptionWrapper works from `outside` the code being run by the tbb scheduler).
    
    //This means that it works for all types supported by boost::exception.
    //Use `boost::enable_current_exception` when throwing exceptions to enable this support.
    //See the boost::exception documentation for more details.
    
    //The copy constructor of A must not throw.
    template<typename F, typename R, typename A>
    class TBBInnerExceptionWrapper
    {
    public:
        TBBInnerExceptionWrapper(F const &f) :
            f_(f)
        {}
    public:
        R operator()(A a) const
        {
            try {
                return f_(a);
            }
            catch (...) {
                throw tbb::movable_exception<std::exception_ptr>(std::current_exception());
            }
        }
    private:
        F f_;
    };

    template<typename F>
    class TBBOuterExceptionWrapper
    {
    public:
        TBBOuterExceptionWrapper(F const &f) :
            f_(f)
        {}
    public:
        void operator()() const
        {
            try {
                f_();
            }
            catch (tbb::movable_exception<std::exception_ptr> const &e) {
                std::rethrow_exception(e.data());
            }
            catch (tbb::tbb_exception const &) {
                //To get here the exception must have originated from outside the InnerExceptionWrapper.
                //The only reasons that that can happen are:
                //  * An exception in the copy-constructor or copy-assignment-operator of `R`.
                //    (`R` would be the return type of `f_`, if this were generalized to non-void returns) 
                //  * An exception originating from within the tbb scheduler
                //In both cases, the only reasonable cause for the exception is a lack of memory,
                //so this sort of exception is translated into a std::bad_alloc
                //(Note that this implies a restriction on R to only throw on out of memory)
                throw std::bad_alloc();
            }
        }
    private:
        F f_;
    };
}
#endif //HG_TBB_EXCEPTION_WRAPPER_H
