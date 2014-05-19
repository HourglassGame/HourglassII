#ifndef HG_MOVE_FUNCTION_H
#define HG_MOVE_FUNCTION_H
#include <utility>
#include <memory>
#include "multi_thread_deleter.h"

#include <functional>

namespace hg {

namespace function {
namespace detail {
template<typename> class function_base;

template<typename R, typename... ArgTypes>
struct function_base<R(ArgTypes...)>
{
    typedef R result_type;
    virtual R operator()(ArgTypes &&...args) = 0;
    virtual ~function_base() noexcept {}
};
template<typename F, typename Signature>
struct function_obj;
template<typename F, typename R, typename... ArgTypes>
struct function_obj<F, R(ArgTypes...)> : function_base<R(ArgTypes...)>
{
    function_obj(F &&f) noexcept :
    	f(std::move(f))
    {
    }
    virtual R operator()(ArgTypes &&...args) override
    {
        return f(std::forward<ArgTypes>(args)...);
    }
private:
    F f;
};
template<typename F, typename... ArgTypes>
struct function_obj<F, void(ArgTypes...)> : function_base<void(ArgTypes...)>
{
    function_obj(F &&f) noexcept :
    	f(std::move(f))
    {
    }
    virtual void operator()(ArgTypes &&...args) override
    {
        f(std::forward<ArgTypes>(args)...);
    }
private:
    F f;
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
	move_function() noexcept = default;
	move_function(move_function &&o) noexcept = default;
    move_function<R(ArgTypes...)> &operator=(move_function &&o) noexcept = default;
    template<typename F>
    move_function(F &&f) :
        f(multi_thread_new<function::detail::function_obj<F, R(ArgTypes...)>>(std::move(f)))
    {
    }
    template<typename F>
    move_function<R(ArgTypes...)> &operator=(F &&f)
    {
        this->f = function_obj_ptr_t(multi_thread_new<function::detail::function_obj<F, R(ArgTypes...)>>(std::move(f)));
        return *this;
    }
    R operator()(ArgTypes &&...args) const {
        return (*f)(std::forward<ArgTypes>(args)...);
    }
    explicit operator bool() noexcept { return f.get(); }
private:
    typedef std::unique_ptr<
        function::detail::function_base<R(ArgTypes...)>,
        multi_thread_deleter<function::detail::function_base<R(ArgTypes...)> > > function_obj_ptr_t;
    function_obj_ptr_t f;
};

} //namespace hg
#endif //HG_MOVE_FUNCTION_H
