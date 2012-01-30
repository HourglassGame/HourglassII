#ifndef HG_TBB_EXCEPTION_WRAPPER_H
#define HG_TBB_EXCEPTION_WRAPPER_H
#include <exception>
#include <tbb/tbb_exception.h>
#include <boost/exception_ptr.hpp>
namespace hg {
    //TBBOuterExceptionWrapper and TBBInnerExceptionWrapper work together
    //to allow exceptions to (slightly more) properly be passed through the 
    //Threading Building Blocks scheduler (in C++03 mode).
    
    //TBBInnerExceptionWrapper wraps all known excepion types up in a tbb::captured_exception
    //with a well defined `name()` field, and then throws them. (TBBInnerExceptionWrapper works from `inside`
    //the code being run by the tbb scheduler -- thus the name).
    //TBBOuterExceptionWrapper catches the `captured_exception` and converts the `name()` field back into an exception
    //which it throws. (TBBOuterExceptionWrapper works from `outside` the code being run by the tbb scheduler).
    //When possible, the `what()` field of the original exception is preserved and passed on to the new exception.
    
    //The copy constructor of A must not throw.
    template<typename F, typename R, typename A>
    class TBBInnerExceptionWrapper
    {
    public:
        TBBInnerExceptionWrapper(F const& f) :
            f_(f)
        {}
    public:
        R operator()(A a) const
        {
            try {
                return f_(a);
            }
            catch (...) {
                throw tbb::movable_exception<boost::exception_ptr>(boost::current_exception());
            }
        }
    private:
        F f_;
    };

    template<typename F>
    class TBBOuterExceptionWrapper
    {
    public:
        TBBOuterExceptionWrapper(F const& f) :
            f_(f)
        {}
    public:
        void operator()() const
        {
            try {
                return f_();
            }
            catch (tbb::movable_exception<boost::exception_ptr> const& e) {
                boost::rethrow_exception(e.data());
            }
            catch (tbb::tbb_exception const&) {
                //To get here the exception must have originated from outside the InnerExceptionWrapper.
                //The only reasons that that can happen are:
                //  * An exception in the copy-constructor or copy-assignment-operator of `R`.
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
