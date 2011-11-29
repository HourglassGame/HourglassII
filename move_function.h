#ifndef HG_MOVE_FUNCTION_H
#define HG_MOVE_FUNCTION_H
#include <utility>
#include <memory>
#include "move.h"
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
    function_obj(F&& f) :
    	f_(hg::move(f))
    {
    }
    function_obj<F, R, ArgTypes...>& operator=(F&& f)
    {
        f_(hg::move(f));
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
    function_obj(F&& f) :
    	f_(hg::move(f))
    {
    }
    function_obj<F, void, ArgTypes...>& operator=(F&& f)
    {
        f_(hg::move(f));
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
	move_function(move_function const&) = delete;
	move_function<R(ArgTypes...)>& operator=(move_function const&) = delete;
	move_function(move_function&& other) :
        f_(hg::move(other.f_))
    {
    }
    move_function<R(ArgTypes...)>& operator=(move_function&& other)
    {
        f_ = hg::move(other.f_);
        return *this;
    }
    template<typename F>
    move_function(F&& f) :
        f_(new function::detail::function_obj<F, R, ArgTypes...>(hg::move(f)))
    {
    }
    template<typename F>
    move_function<R(ArgTypes...)>& operator=(F&& f)
    {
        f_ = hg::unique_ptr<function::detail::function_base<R(ArgTypes...)>>(
            new function::detail::function_obj<F, R, ArgTypes...>(hg::move(f)));
        return *this;
    }
    R operator()(ArgTypes&&... args) {
        return (*f_)(hg::forward<ArgTypes>(args)...);
    }
private:
    hg::unique_ptr<function::detail::function_base<R(ArgTypes...)>> f_;
};
} //namespace hg
#endif //HG_MOVE_FUNCTION_H
