#ifndef HG_MOVE_FUNCTION_H
#define HG_MOVE_FUNCTION_H
#include <utility>
#include <memory>
#include <boost/move/move.hpp>
#include "forward.h"
#include "unique_ptr.h"
namespace hg {

namespace function {
namespace detail {
template<typename> class function_base;
template<typename R, typename... ArgTypes>
struct function_base<R(ArgTypes...)>
{
    virtual R operator()(ArgTypes&&... args) = 0;
    virtual ~function_base() {}
};
template<typename F, typename R, typename... ArgTypes>
struct function_obj : function_base<R(ArgTypes...)>
{
    function_obj(BOOST_RV_REF(F) f) :
    	f_(hg::move(f))
    {
    }
    function_obj<F, R, ArgTypes...>& operator=(BOOST_RV_REF(F) f)
    {
        f_(boost::move(f));
        return *this;
    }
    virtual R operator()(ArgTypes&&... args)
    {
        return f_(hg::forward<ArgTypes>(args)...);
    }
private:
    F f_;
};
template<typename F, typename... ArgTypes>
struct function_obj<F, void, ArgTypes...> : function_base<void(ArgTypes...)>
{
    function_obj(BOOST_RV_REF(F) f) :
    	f_(boost::move(f))
    {
    }
    function_obj<F, void, ArgTypes...>& operator=(BOOST_RV_REF(F) f)
    {
        f_(boost::move(f));
        return *this;
    }
    virtual void operator()(ArgTypes&&... args)
    {
        f_(hg::forward<ArgTypes>(args)...);
    }
private:
    F f_;
};
}
}

//A version of std::function which
//allows the use of movable but 
//non-copyable functors. The trade-off
//is that copying of move_function is
//disallowed (because copying would only be 
//possible for `move_functions` that were constructed
//with copyable functors).
template<typename> class move_function;
template<typename R, typename... ArgTypes>
class move_function<R(ArgTypes...)>
{
public:
	move_function() : f_() {}
	move_function(BOOST_RV_REF(move_function) other) :
        f_(boost::move(other.f_))
    {
    }
    move_function<R(ArgTypes...)>& operator=(BOOST_RV_REF(move_function) other)
    {
        f_ = boost::move(other.f_);
        return *this;
    }
    template<typename F>
    move_function(BOOST_RV_REF(F) f) :
        f_(new function::detail::function_obj<F, R, ArgTypes...>(boost::move(f)))
    {
    }
    template<typename F>
    move_function<R(ArgTypes...)>& operator=(BOOST_RV_REF(F) f)
    {
        f_ = hg::unique_ptr<function::detail::function_base<R(ArgTypes...)> >(
            new function::detail::function_obj<F, R, ArgTypes...>(boost::move(f)));
        return *this;
    }
    R operator()(ArgTypes&&... args) {
        return (*f_)(hg::forward<ArgTypes>(args)...);
    }
private:
    hg::unique_ptr<function::detail::function_base<R(ArgTypes...)> > f_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(move_function)
};
} //namespace hg
#endif //HG_MOVE_FUNCTION_H
