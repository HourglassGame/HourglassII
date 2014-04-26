#ifndef HG_CLONE_PTR_H
#define HG_CLONE_PTR_H
#include <boost/swap.hpp>
#include <cassert>
#include "default_delete.h"
#include "default_clone.h"
namespace hg {
//It is desirable for this to be a smart reference
//(rather than a smart pointer)
//but this is not possible in C++ ):
template<typename Cloneable, typename CloneManager = default_clone<Cloneable> >
class clone_ptr : CloneManager
{
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
    clone_ptr(clone_ptr const &o) :
        CloneManager(),
        obj(o.obj?CloneManager::new_clone(*o.obj):nullptr)
    {
    }
    clone_ptr &operator=(clone_ptr const &o)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(o);
    }
    clone_ptr(clone_ptr &&o) :
    	CloneManager(std::move(static_cast<CloneManager &>(o))), obj(o.obj)
    {
    	o.obj = nullptr;
    }
    clone_ptr &operator=(clone_ptr &&o)
	{
		swap(o);
		return *this;
	}
    void swap(clone_ptr &o) {
        boost::swap(obj, o.obj);
        boost::swap(static_cast<CloneManager &>(*this), static_cast<CloneManager &>(o));
    }
    ~clone_ptr() {
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
inline void swap(clone_ptr<Cloneable, CloneManager> &l, clone_ptr<Cloneable, CloneManager> &r) { l.swap(r); }
}
#endif //HG_CLONE_PTR_H
