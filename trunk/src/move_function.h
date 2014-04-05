#ifndef HG_MOVE_FUNCTION_H
#define HG_MOVE_FUNCTION_H
#include <utility>
#include <memory>
#include <boost/move/move.hpp>
#include <boost/config.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/decay.hpp>
#include "multi_thread_deleter.h"
#include "forward.h"

#ifdef BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
# define EXPLICIT_OPERATOR_BOOL(expression)                                            \
private:                                                                               \
    struct safe_bool_dummy { void nonnull() {} };                                      \
    typedef void (safe_bool_dummy::*safe_bool)();                                      \
public:                                                                                \
    operator safe_bool() const { return (expression) ? &safe_bool_dummy::nonnull : 0; }
#else
# define EXPLICIT_OPERATOR_BOOL(expression) \
    explicit operator bool() const { return (expression); }
#endif


namespace hg {

namespace function {
namespace detail {
template<typename> class function_base;

template<typename R, typename... ArgTypes>
struct function_base<R(ArgTypes...)>
{
    virtual R operator()(ArgTypes &&...args) = 0;
    virtual ~function_base() {}
};
template<typename F, typename R, typename... ArgTypes>
struct function_obj : function_base<R(ArgTypes...)>
{
    function_obj(BOOST_RV_REF(F) f) :
    	f_(boost::move(f))
    {
    }
    function_obj<F, R, ArgTypes...>& operator=(BOOST_RV_REF(F) f)
    {
        f_ = boost::move(f);
        return *this;
    }
    virtual R operator()(ArgTypes &&...args)
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
        f_ = boost::move(f);
        return *this;
    }
    virtual void operator()(ArgTypes &&...args)
    {
        f_(hg::forward<ArgTypes>(args)...);
    }
private:
    F f_;
};


template<typename R>
struct function_base<R()>
{
	typedef R result_type;
    virtual R operator()() = 0;
    virtual ~function_base() {}
};

template<typename Functor, typename Signature>
struct move_function_obj : function_base<Signature>
{
    move_function_obj(BOOST_RV_REF(Functor) f) :
    	f_(boost::move(f))
    {
    }
    move_function_obj &operator=(BOOST_RV_REF(Functor) f)
    {
        f_ = boost::move(f);
        return *this;
    }
    virtual typename function_base<Signature>::result_type operator()()
    {
        return f_();
    }
private:
    Functor f_;
};

template<typename Functor>
struct move_function_obj<Functor, void()> : function_base<void()>
{
    move_function_obj(BOOST_RV_REF(Functor) f) :
    	f_(boost::move(f))
    {
    }
    move_function_obj &operator=(BOOST_RV_REF(Functor) f)
    {
        f_= boost::move(f);
        return *this;
    }
    virtual void operator()()
    {
        f_();
    }
private:
    Functor f_;
};
/*
template<typename Functor, typename Signature>
struct function_obj : function_base<Signature>
{
    function_obj(Functor const &f) :
    	f_(f)
    {
    }
    function_obj &operator=(Functor const &f)
    {
        f_ = f;
        return *this;
    }
    virtual typename function_base<Signature>::result_type operator()()
    {
        return f_();
    }
private:
    Functor f_;
};

template<typename Functor>
struct function_obj<Functor, void()> : function_base<void()>
{
    function_obj(Functor const &f) :
    	f_(f)
    {
    }
    function_obj &operator=(Functor const &f)
    {
        f_(f);
        return *this;
    }
    virtual void operator()()
    {
        f_();
    }
private:
    Functor f_;
};
*/
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
	move_function(BOOST_RV_REF(move_function) o) :
        f_(boost::move(o.f_))
    {
    }
    move_function<R(ArgTypes...)>& operator=(BOOST_RV_REF(move_function) o)
    {
        f_ = boost::move(o.f_);
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
        f_ = std::unique_ptr<function::detail::function_base<R(ArgTypes...)> >(
            new function::detail::function_obj<F, R, ArgTypes...>(boost::move(f)));
        return *this;
    }
    R operator()(ArgTypes &&...args) const {
        return (*f_)(hg::forward<ArgTypes>(args)...);
    }
private:
    std::unique_ptr<function::detail::function_base<R(ArgTypes...)> > f_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(move_function)
};


template<typename R>
class move_function<R()>
{
public:
	move_function() : f_() {}
	move_function(BOOST_RV_REF(move_function) o) :
        f_(boost::move(o.f_))
    {
    }
    move_function<R()>& operator=(BOOST_RV_REF(move_function) o)
    {
        f_ = boost::move(o.f_);
        return *this;
    }
#ifdef BOOST_NO_RVALUE_REFERENCES
    template<typename F>
    move_function(BOOST_RV_REF(F) f, typename boost::enable_if<boost::has_move_emulation_enabled<F> >::type *p=0) :
        f_(multi_thread_new<function::detail::move_function_obj<F, R()>, boost::rv<F>&>(boost::move(f)))
    {
    }

    template<typename F>
    typename boost::enable_if<boost::has_move_emulation_enabled<F>, move_function<void()>&>::type
    operator=(BOOST_RV_REF(F) f)
    {
        unique_ptr_t(multi_thread_new<function::detail::move_function_obj<F, R()> >(boost::move(f))).swap(f_);
        return *this;
    }

    template<typename F>
    move_function(F f, typename boost::disable_if<boost::has_move_emulation_enabled<F> >::type *p=0) :
        f_(multi_thread_new<function::detail::function_obj<F, R()> >(f))
    {
    }

    template<typename F>
    typename boost::disable_if<boost::has_move_emulation_enabled<F>, move_function<void()>&>::type
    operator=(F f)
    {
        unique_ptr_t(multi_thread_new<function::detail::function_obj<F, R()> >(f)).swap(f_);
        return *this;
    }
#else
    template<typename F>
    move_function(F &&f) :
        f_(multi_thread_new<function::detail::move_function_obj<typename boost::decay<F>::type, R()> >(hg::forward<F>(f)))
    {
    }

    template<typename F>
    move_function<R()>& operator=(F &&f)
    {
        unique_ptr_t(multi_thread_new<function::detail::move_function_obj<typename boost::decay<F>::type, R()> >(hg::forward<F>(f))).swap(f_);
        return *this;
    }
#endif

    R operator()() const {
        return (*f_)();
    }

    EXPLICIT_OPERATOR_BOOL(!empty())

    bool empty() const { return !f_.get(); }

private:
    typedef boost::interprocess::unique_ptr<
        function::detail::function_base<R()>,
        multi_thread_deleter<function::detail::function_base<R()> > > unique_ptr_t;
    unique_ptr_t f_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(move_function)
};
} //namespace hg
#endif //HG_MOVE_FUNCTION_H
