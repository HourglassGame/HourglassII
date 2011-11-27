#ifndef HG_CLONE_PTR_H
#define HG_CLONE_PTR_H
#include <boost/swap.hpp>
#include <cassert>
namespace hg {
//It is desirable for this to be a smart reference
//(rather than a smart pointer)
//but this is not possible in C++ ):
template<typename Cloneable>
class clone_ptr
{
public:
    clone_ptr() :
    obj(0)
    {
    }
    //clone_ptr takes ownership of p
    //(that is, it does not clone p)
    explicit clone_ptr(Cloneable* p) :
        obj(p)
    {
        assert(obj);
    }
    clone_ptr(clone_ptr const& other) :
        obj(new_clone(*other.obj))
    {
    }
    clone_ptr& operator=(clone_ptr const& other)
    {
        return *this = clone_ptr<Cloneable>(other);
    }
    clone_ptr(clone_ptr&& other) :
    	obj(0)
    {
    	boost::swap(obj, other.obj);
    }
    clone_ptr& operator=(clone_ptr&& other)
	{
		boost::swap(obj, other.obj);
		return *this;
	}
    void swap(clone_ptr& other) {
        boost::swap(obj, other.obj);
    }
    ~clone_ptr() {
        delete_clone(obj);
    }
    Cloneable& get() {
        assert(obj);
        return *obj;
    }
    Cloneable const& get() const {
        assert(obj);
        return *obj;
    }
    
private:
    Cloneable* obj;
};
template <typename Cloneable>
inline void swap(clone_ptr<Cloneable>& l, clone_ptr<Cloneable>& r) { l.swap(r); }
}
#endif //HG_CLONE_PTR_H
