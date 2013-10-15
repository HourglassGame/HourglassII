#ifndef HG_UNIQUE_PTR_H
#define HG_UNIQUE_PTR_H
#include <boost/move/move.hpp>
#include <boost/checked_delete.hpp>
#include <boost/type_traits/add_lvalue_reference.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/swap.hpp>
#include "default_delete.h"

//This is not a standard conforming unique_ptr implementation!!
//However, it can be modified to be one if that is required.
//I have simply not been bothered to implement
//all the required functionality.
namespace hg {
namespace unique_ptr_detail {
    namespace has_pointer_type_imp
    {
        template <class T> static long test(...);
        template <class T> static char test(typename T::pointer * = 0);
    }

    template <class T>
    struct has_pointer_type
        : public boost::integral_constant<bool, sizeof has_pointer_type_imp::test<T>(0) == 1>
    {
    };

    namespace pointer_type_imp
    {
        template <class T, class D, bool = has_pointer_type<D>::value>
        struct pointer_type
        {
            typedef typename D::pointer type;
        };

        template <class T, class D>
        struct pointer_type<T, D, false>
        {
            typedef T *type;
        };
    }

    template <class T, class D>
    struct pointer_type
    {
        typedef typename pointer_type_imp::pointer_type<T, typename boost::remove_reference<D>::type>::type type;
    };
}


template<class T, class D = default_delete<T> > class unique_ptr : D
{
public:
    typedef T element_type;
    typedef D deleter_type;
    typedef typename unique_ptr_detail::pointer_type<T, D>::type pointer;
    unique_ptr() :
        D(),
        p()
    {}
    explicit unique_ptr(pointer p) :
        D(),
        p(p)
    {
    }
    unique_ptr(pointer p, const D &d) :
        D(d),
        p(p)
    {
    }
    unique_ptr(pointer p, BOOST_RV_REF(D) d) :
        D(boost::move(d)),
        p(p)
    {
    }
    unique_ptr(BOOST_RV_REF(unique_ptr) u) :
        D(boost::move(u.get_deleter())),
        p(u.p)
    {
        u.p = pointer();
    }
    template <class U, class E>
    unique_ptr(BOOST_RV_REF_2_TEMPL_ARGS(unique_ptr, U, E) u) :
        D(boost::move(u.D)),
        p(u.p)
    {
        u.p = pointer();
    }
    ~unique_ptr()
    {
        if (p) get_deleter()(p);
    }
    unique_ptr &operator=(BOOST_RV_REF(unique_ptr) u)
    {
        reset(u.release());
        get_deleter() = boost::move(u.get_deleter());
        return *this;
    }
    template <class U, class E> unique_ptr &operator=(BOOST_RV_REF_2_TEMPL_ARGS(unique_ptr, U, E) u)
    {
        reset(u.release());
        get_deleter() = boost::move(u.get_deleter());
        return *this;
    }
    typename boost::add_lvalue_reference<T>::type operator*() const
    {
        return *p;
    }
    pointer operator->() const
    {
        return p;
    }
    pointer get() const
    {
        return p;
    }
    deleter_type &get_deleter() { return *this; }
    deleter_type const &get_deleter() const { return *this; }
    pointer release()
    {
        pointer retv(p);
        p = pointer();
        return retv;
    }
    void reset(pointer p = pointer())
    {
        pointer old_p(this->p);
        this->p = p;
        if (old_p) get_deleter()(old_p);
    }
    void swap(unique_ptr &u)
    {
        boost::swap(p, u.p);
        boost::swap(get_deleter(), u.get_deleter());
    }

private:
    pointer p;
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(unique_ptr)
};
template<class T> class unique_ptr<T[]>;
template<typename T> void swap(unique_ptr<T>& l, unique_ptr<T>& r)
{
    l.swap(r);
}
template<typename T1, typename T2>
bool operator==(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() == r.get();
}
template<typename T1, typename T2>
bool operator!=(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() != r.get();
}
template<typename T1, typename T2>
bool operator<(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() < r.get();
}
template<typename T1, typename T2>
bool operator<=(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() <= r.get();
}
template<typename T1, typename T2>
bool operator>(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() > r.get();
}
template<typename T1, typename T2>
bool operator>=(unique_ptr<T1> const &l, unique_ptr<T2> const &r)
{
    return l.get() >= r.get();
}
}
#endif //HG_UNIQUE_PTR_H
