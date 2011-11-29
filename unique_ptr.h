#ifndef HG_UNIQUE_PTR_H
#define HG_UNIQUE_PTR_H
#include "move.h"
#include <boost/checked_delete.hpp>

//This is not a standard conforming unique_ptr implementation!!
//However, it can be modified to be one if that is required.
//I have simply not been bothered to implement
//all the required functionality.
namespace hg {
template<class T> class unique_ptr
{
public:
    constexpr unique_ptr() :
        ptr_(nullptr)
    {}
    explicit unique_ptr(T* p) :
        ptr_(p)
    {
    }
    unique_ptr(unique_ptr&& u) :
        ptr_(u.ptr_)
    {
        u.ptr_ = nullptr;
    }
    template <class U>
    unique_ptr(unique_ptr<U>&& u) :
        ptr_(u.ptr_)
    {
        u.ptr_ = nullptr;
    }
    ~unique_ptr()
    {
        boost::checked_delete(ptr_);
    }
    unique_ptr& operator=(unique_ptr&& u)
    {
        boost::swap(ptr_, u.ptr_);
        return *this;
    }
    template <class U> unique_ptr& operator=(unique_ptr<U>&& u)
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
    explicit operator bool() const
    {
        return static_cast<bool>(ptr_);
    }
    T* release()
    {
        T* retv(ptr_);
        ptr_ = nullptr;
        return retv;
    }
    void reset(T* p = nullptr)
    {
        checked_delete(ptr_);
        ptr_ = p;
    }
    void swap(unique_ptr& u)
    {
        boost::swap(ptr_, u.ptr_);
    }
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
private:
    T* ptr_;
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
