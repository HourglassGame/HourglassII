#ifndef HG_CLONE_PTR_H
#define HG_CLONE_PTR_H
#include <boost/swap.hpp>
#include <boost/move/move.hpp>
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
        obj(0)
    {
    }
    //clone_ptr takes ownership of p
    //(that is, it does not clone p)
    explicit clone_ptr(Cloneable* p) :
        CloneManager(),
        obj(p)
    {
        assert(obj);
    }
    clone_ptr(clone_ptr const& other) :
        CloneManager(),
        obj(other.obj?CloneManager::new_clone(*other.obj):0)
    {
    }
    clone_ptr& operator=(BOOST_COPY_ASSIGN_REF(clone_ptr) other)
    {
        //Forward to move assignment operator
        return *this = clone_ptr(other);
    }
    clone_ptr(BOOST_RV_REF(clone_ptr) other) :
    	obj(0)
    {
    	boost::swap(obj, other.obj);
    }
    clone_ptr& operator=(BOOST_RV_REF(clone_ptr) other)
	{
		boost::swap(obj, other.obj);
		return *this;
	}
    void swap(clone_ptr& other) {
        boost::swap(obj, other.obj);
    }
    ~clone_ptr() {
        CloneManager::delete_clone(obj);
    }
    Cloneable& get() {
        assert(obj);
        return *obj;
    }
    Cloneable const& get() const {
        assert(obj);
        return *obj;
    }
    Cloneable* operator->() const {
        return obj;
    }
    Cloneable& operator*() const {
        return *obj;
    }
private:
    Cloneable* obj;
    BOOST_COPYABLE_AND_MOVABLE(clone_ptr)
};
template <typename Cloneable, typename CloneManager>
inline void swap(clone_ptr<Cloneable, CloneManager>& l, clone_ptr<Cloneable, CloneManager>& r) { l.swap(r); }
}
#endif //HG_CLONE_PTR_H
