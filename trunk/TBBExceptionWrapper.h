#ifndef HG_TBB_EXCEPTION_WRAPPER_H
#define HG_TBB_EXCEPTION_WRAPPER_H
#include <exception>
#include <tbb/tbb_exception.h>
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
            catch (tbb::captured_exception const& e) {
#define HG_RETHROW_WRAPPED_WHAT_EXCEPTION(exception_type)   \
    if (strcmp(e.name(), "hg_" #exception_type) == 0) {     \
        throw exception_type(e.what());                     \
    }
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::domain_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::invalid_argument)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::length_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::out_of_range)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::logic_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::range_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::overflow_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::underflow_error)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::ios_base::failure)
                HG_RETHROW_WRAPPED_WHAT_EXCEPTION(std::runtime_error)
#undef HG_RETHROW_WRAPPED_WHAT_EXCEPTION
#define HG_RETHROW_WRAPPED_EXCEPTION(exception_type)    \
    if (strcmp(e.name(), "hg_" #exception_type) == 0) { \
        throw exception_type();                         \
    }
                HG_RETHROW_WRAPPED_EXCEPTION(std::bad_alloc)
                HG_RETHROW_WRAPPED_EXCEPTION(std::bad_cast)
                HG_RETHROW_WRAPPED_EXCEPTION(std::bad_typeid)
                HG_RETHROW_WRAPPED_EXCEPTION(std::bad_exception)
                HG_RETHROW_WRAPPED_EXCEPTION(std::exception)
#undef HG_RETHROW_WRAPPED_EXCEPTION
                //To get here the tbb::captured_exception must have originated from somewhere
                //that was not wrapped by TBBInnerExceptionWrapper.
                //This can only occur if the exception originated from within the scheduler.
                //The only reason that the scheduler should ever fail is lack of memory, so 
                //std::bad_alloc is the appropriate exception to rethrow.
                //(Note that a variety of easy to make bugs could also result in getting here,
                // if any of these bugs are occuring then fix them!)
                throw std::bad_alloc();
            }
        }
    private:
        F f_;
    };
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
#define HG_WRAP_EXCEPTION(exception_type)                               \
    catch (exception_type const& e) {                                   \
        throw tbb::captured_exception("hg_" #exception_type, e.what());  \
    }
            HG_WRAP_EXCEPTION(std::domain_error)
            HG_WRAP_EXCEPTION(std::invalid_argument)
            HG_WRAP_EXCEPTION(std::length_error)
            HG_WRAP_EXCEPTION(std::out_of_range)
            HG_WRAP_EXCEPTION(std::logic_error)
            HG_WRAP_EXCEPTION(std::range_error)
            HG_WRAP_EXCEPTION(std::overflow_error)
            HG_WRAP_EXCEPTION(std::underflow_error)
            HG_WRAP_EXCEPTION(std::ios_base::failure)
            HG_WRAP_EXCEPTION(std::runtime_error)
            HG_WRAP_EXCEPTION(std::bad_alloc)
            HG_WRAP_EXCEPTION(std::bad_cast)
            HG_WRAP_EXCEPTION(std::bad_typeid)
            HG_WRAP_EXCEPTION(std::bad_exception)
            HG_WRAP_EXCEPTION(std::exception)
#undef HG_RETHROW_WRAPPED_EXCEPTION
            catch (...) {
                assert(false &&
                    "An unknown exception was thrown through TBBInnerExceptionWrapper."
                    " Please add support for this exception type.");
            }
        }
    private:
        F f_;
    };
    
}
#endif //HG_TBB_EXCEPTION_WRAPPER_H
