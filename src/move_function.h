#ifndef HG_MOVE_FUNCTION_H
#define HG_MOVE_FUNCTION_H
#include <utility>
#include <memory>
#include "multi_thread_deleter.h"
#include <cassert>
#include <type_traits>
#include <functional>

namespace hg {

namespace function {
namespace detail {
template<typename> struct function_base;

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
        f(std::forward<F>(f))
    {
    }
    virtual R operator()(ArgTypes &&...args) override
    {
        return std::forward<F>(f)(std::forward<ArgTypes>(args)...);
    }
private:
    typename std::decay<F>::type f;
};
template<typename F, typename... ArgTypes>
struct function_obj<F, void(ArgTypes...)> : function_base<void(ArgTypes...)>
{
    function_obj(F &&f) noexcept :
        f(std::forward<F>(f))
    {
    }
    virtual void operator()(ArgTypes &&...args) override
    {
        std::forward<F>(f)(std::forward<ArgTypes>(args)...);
    }
private:
    typename std::decay<F>::type f;
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
        f(new (multi_thread_tag{}) function::detail::function_obj<F, R(ArgTypes...)>(std::forward<F>(f)))
    {
    }
    template<typename F>
    move_function<R(ArgTypes...)> &operator=(F &&f)
    {
        this->f = function_obj_ptr_t(
            new (multi_thread_tag{}) function::detail::function_obj<F, R(ArgTypes...)>(std::forward<F>(f)));
        return *this;
    }
    R operator()(ArgTypes &&...args) const {
        assert(f);
        return (*f)(std::forward<ArgTypes>(args)...);
    }
    explicit operator bool() const noexcept { return f.get(); }

private:
    typedef std::unique_ptr<
        function::detail::function_base<R(ArgTypes...)>,
        multi_thread_deleter<function::detail::function_base<R(ArgTypes...)>>> function_obj_ptr_t;
    function_obj_ptr_t f;
};

} //namespace hg
#endif //HG_MOVE_FUNCTION_H
