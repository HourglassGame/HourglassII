#ifndef HG_SINGLE_ASSIGNMENT_PTR_H
#define HG_SINGLE_ASSIGNMENT_PTR_H
#include "multi_thread_allocator.h"
#include <cassert>
namespace hg {
//Very sad workaround for the limited support for rv-refs
//that we are working with.
//This type has two states: uninitialised, and initialised.
//Only uninitialised instances may be assigned to.
//The result of assigning to initialised instances is undefined.
//The idea is to support 2-stage construcion for non-copyable types to allow them to be
//inserted into containers.
//This is an ugle kludge, but it must do until the bugs in Boost.Container are fixed.
//If it gets too bad I will fix those bugs myself.
template<typename T>
class SingleAssignmentPtr {
public:
    SingleAssignmentPtr() : ptr_(0) {}
    SingleAssignmentPtr(SingleAssignmentPtr<T> const& o) : ptr_(0) { assert(!o.ptr_); }
    //p must have been allocated using multi_thread_operator_new.
    SingleAssignmentPtr(T* p) : ptr_(p) {}
    SingleAssignmentPtr& operator=(SingleAssignmentPtr<T> o) { assert(!(ptr_ || o.ptr_)); return *this; }
    SingleAssignmentPtr& operator=(T* p) { assert(!ptr_); ptr_ = p; return *this;}
    ~SingleAssignmentPtr() { if(ptr_) { ptr_->~T(); multi_thread_operator_delete(ptr_); } }
    T* get() const { return ptr_; }
    T* operator->() const { return ptr_; }
    T& operator*() const { return assert(ptr_), *ptr_; }
private:
    T* ptr_;
};
}
#endif //HG_SINGLE_ASSIGNMENT_PTR_H
