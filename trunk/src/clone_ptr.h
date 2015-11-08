#ifndef HG_CLONE_PTR_H
#define HG_CLONE_PTR_H
#include <boost/swap.hpp>
#include <cassert>
#include "default_clone.h"
namespace hg {
//It is desirable for this to be a smart reference
//(rather than a smart pointer)
//but this is not possible in C++ ):

//TODO -- add complete support for stateful CloneManager.
//        support is currently incomplete and buggy.
template<typename Cloneable, typename CloneManager = default_clone<Cloneable>>
class clone_ptr : CloneManager
{
    template<typename, typename> friend class clone_ptr;
public:
    typedef Cloneable *pointer;

    clone_ptr() :
        CloneManager(),
        obj()
    {
    }
    //clone_ptr takes ownership of p
    //(that is, it does not clone p)
    explicit clone_ptr(Cloneable *p) :
        CloneManager(),
        obj(p)
    {
    }
    
    //TODO -- a constructor allowing initiaisation of CloneManager
    
    clone_ptr(clone_ptr const &o) :
        CloneManager(o.get_cloner()),
        obj(o.obj?CloneManager::new_clone(*o.obj):pointer())
    {
    }
    template<typename U, typename E>
    clone_ptr(clone_ptr<U,E> const &o) :
        CloneManager(std::forward<E>(o.get_cloner())),
        obj(o.obj?CloneManager::new_clone(*o.obj):pointer())
    {
    }
    
    clone_ptr &operator=(clone_ptr const &o)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(o);
    }
    template<typename U, typename E>
    clone_ptr &operator=(clone_ptr<U, E> const &o)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(o);
    }
    
    clone_ptr(clone_ptr &&o) noexcept :
        CloneManager(std::move(o.get_cloner())), obj(o.obj)
    {
        o.obj = pointer();
    }
    template<typename U, typename E>
    clone_ptr(clone_ptr<U,E> &&o) noexcept :
        CloneManager(std::forward<E>(o.get_cloner())), obj(o.obj)
    {
        o.obj = typename clone_ptr<U,E>::pointer();
    }
    
    clone_ptr &operator=(clone_ptr &&o) noexcept
    {
        swap(o);
        return *this;
    }
    template<typename U, typename E>
    clone_ptr &operator=(clone_ptr<U, E> &&o) noexcept
    {
        static_assert(
            std::is_nothrow_move_assignable<CloneManager>::value,
            "CloneManager must be no-throw move assignable to call operator=");
        
        reset(o.release());
        get_cloner() = std::forward<E>(o.get_cloner());
        return *this;
    }
    
    void swap(clone_ptr &o) noexcept {
        boost::swap(obj, o.obj);
        boost::swap(get_cloner(), o.get_cloner());
    }
    ~clone_ptr() noexcept {
        CloneManager::delete_clone(obj);
    }
    
    void reset(pointer ptr = pointer()) noexcept {
        pointer old_ptr = obj;
        obj = ptr;
        if (old_ptr) get_cloner().delete_clone(old_ptr);
    }
    
    Cloneable *get() noexcept {
        return obj;
    }
    Cloneable const &get() const noexcept {
        assert(obj);
        return *obj;
    }
    Cloneable *operator->() const noexcept {
        assert(obj);
        return obj;
    }
    Cloneable &operator*() const noexcept {
        return *obj;
    }
    CloneManager &get_cloner() noexcept {
        return *this;
    }
    CloneManager const &get_cloner() const noexcept {
        return *this;
    }
private:
    Cloneable *obj;
};
template <typename Cloneable, typename CloneManager>
inline void swap(clone_ptr<Cloneable, CloneManager> &l, clone_ptr<Cloneable, CloneManager> &r) noexcept { l.swap(r); }

template<typename T, typename... Args>
clone_ptr<T> make_clone_ptr(Args &&...args) {
    return clone_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif //HG_CLONE_PTR_H
