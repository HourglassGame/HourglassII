#ifndef HG_UNIQUE_PTR_H
#define HG_UNIQUE_PTR_H
#include <boost/move/move.hpp>
#include <boost/checked_delete.hpp>
#include <boost/type_traits/add_lvalue_reference.hpp>
#include <boost/swap.hpp>

//This is not a standard conforming unique_ptr implementation!!
//However, it can be modified to be one if that is required.
//I have simply not been bothered to implement
//all the required functionality.
namespace hg {
template<class T> class unique_ptr
{
public:
    unique_ptr() :
        ptr_(0)
    {}
    explicit unique_ptr(T* p) :
        ptr_(p)
    {
    }
    unique_ptr(BOOST_RV_REF(unique_ptr) u) :
        ptr_(u.ptr_)
    {
        u.ptr_ = 0;
    }
    template <class U>
    unique_ptr(BOOST_RV_REF(unique_ptr<U>) u) :
        ptr_(u.ptr_)
    {
        u.ptr_ = 0;
    }
    ~unique_ptr()
    {
        boost::checked_delete(ptr_);
    }
    unique_ptr& operator=(BOOST_RV_REF(unique_ptr) u)
    {
        boost::swap(ptr_, u.ptr_);
        return *this;
    }
    template <class U> unique_ptr& operator=(BOOST_RV_REF(unique_ptr<U>) u)
    {
        boost::swap(ptr_, u.ptr_);
        return *this;
    }
    typename boost::add_lvalue_reference<T>::type operator*() const
    {
        return *ptr_;
    }
    T* operator->() const
    {
        return ptr_;
    }
    T* get() const
    {
        return ptr_;
    }
    T* release()
    {
        T* retv(ptr_);
        ptr_ = 0;
        return retv;
    }
    void reset(T* p = 0)
    {
        boost::checked_delete(ptr_);
        ptr_ = p;
    }
    void swap(unique_ptr& u)
    {
        boost::swap(ptr_, u.ptr_);
    }

private:
    T* ptr_;
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(unique_ptr)
};
template<typename T> class unique_ptr<T[]>;
template<typename T> void swap(unique_ptr<T>& l, unique_ptr<T>& r)
{
    l.swap(r);
}
template<typename T1, typename T2>
bool operator==(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() == r.get();
}
template<typename T1, typename T2>
bool operator!=(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() != r.get();
}
template<typename T1, typename T2>
bool operator<(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() < r.get();
}
template<typename T1, typename T2>
bool operator<=(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() <= r.get();
}
template<typename T1, typename T2>
bool operator>(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() > r.get();
}
template<typename T1, typename T2>
bool operator>=(unique_ptr<T1> const& l, unique_ptr<T2> const& r)
{
    return l.get() >= r.get();
}
}
#endif //HG_UNIQUE_PTR_H
