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
    template<typename, typename>
    friend class clone_ptr;
public:
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
        CloneManager(),
        obj(o.obj?CloneManager::new_clone(*o.obj):nullptr)
    {
    }
    template<typename U>
    clone_ptr(clone_ptr<U> const &o) :
        CloneManager(),
        obj(o.obj?CloneManager::new_clone(*o.obj):nullptr)  //TODO -- sensible CloneManager behaviour when moving/copying from subtype
    {
    }
    
    clone_ptr &operator=(clone_ptr const &o)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(o);
    }
    template<typename U>
    clone_ptr &operator=(clone_ptr<U> const &o)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(o);
    }
    
    clone_ptr(clone_ptr &&o) noexcept :
    	CloneManager(std::move(static_cast<CloneManager &>(o))), obj(o.obj)
    {
    	o.obj = nullptr;
    }
    template<typename U>
    clone_ptr(clone_ptr<U> &&o) noexcept :
    	CloneManager(), obj(o.obj) //TODO -- sensible CloneManager behaviour when moving/copying from subtype
    {
    	o.obj = nullptr;
    }
    
    clone_ptr &operator=(clone_ptr &&o) noexcept
	{
		swap(o);
		return *this;
	}
    template<typename U>
    clone_ptr &operator=(clone_ptr<U> &&o) noexcept
	{
        //TODO -- sensible CloneManager behaviour when moving/copying from subtype
        CloneManager::delete_clone(obj);
        obj = o.obj;
        o.obj = nullptr;
		return *this;
	}
    
    void swap(clone_ptr &o) noexcept {
        boost::swap(obj, o.obj);
        boost::swap(static_cast<CloneManager &>(*this), static_cast<CloneManager &>(o));
    }
    ~clone_ptr() noexcept {
        CloneManager::delete_clone(obj);
    }
    Cloneable *get() {
        return obj;
    }
    Cloneable const &get() const {
        assert(obj);
        return *obj;
    }
    Cloneable *operator->() const {
        assert(obj);
        return obj;
    }
    Cloneable &operator*() const {
        return *obj;
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
