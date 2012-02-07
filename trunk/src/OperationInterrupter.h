#ifndef HG_OPERATION_interrupter_H
#define HG_OPERATION_interrupter_H
#include <tbb/spin_mutex.h>
#include "mt/boost/container/stable_vector.hpp"
#include "move_function.h"
#include <boost/swap.hpp>
#include <boost/move/move.hpp>

namespace hg {
class OperationInterrupter {
public:
    //Keeps the added interruption function
    class FunctionHandle {
    public:
        FunctionHandle() : interrupter_(), iterator_() {}
        FunctionHandle(
            OperationInterrupter& interrupter,
            mt::boost::container::stable_vector<move_function<void()> >::type::iterator iterator)
                : interrupter_(&interrupter), iterator_(iterator) {}
        
        FunctionHandle(BOOST_RV_REF(FunctionHandle) o) :
            interrupter_(), iterator_()
        {
            swap(o);
        }
        
        FunctionHandle& operator=(BOOST_RV_REF(FunctionHandle) o)
        {
            swap(o);
            return *this;
        }
        
        void swap(FunctionHandle& o) {
            boost::swap(interrupter_, o.interrupter_);
            boost::swap(iterator_, o.iterator_);
        }
        
        ~FunctionHandle() {
            if (interrupter_) {
                OperationInterrupter& i(*interrupter_);
                tbb::spin_mutex::scoped_lock lock(i.mutex_);
                i.interruptionFunctions_.erase(iterator_);
            }
        }
    private:
        OperationInterrupter* interrupter_;
        mt::boost::container::stable_vector<move_function<void()> >::type::iterator iterator_;
        BOOST_MOVABLE_BUT_NOT_COPYABLE(FunctionHandle)
    };
    
    //Calls all currently active interruptionFunctions
    //Causes attempts to add new interruptionFunctions to immediately call
    //the passed interruptionFunction.
	void interrupt();
    
    //While the returned FunctionHandle (or a FunctionHandle moved from the returned FunctionHandle) is alive,
    //a call to `interrupt` will call interruptionFunction. The returned FunctionHandle must not outlive
    //the OperationInterrupter.
	FunctionHandle addInterruptionFunction(move_function<void()> interruptionFunction);

private:
	bool interrupted_;
	mt::boost::container::stable_vector<move_function<void()> >::type interruptionFunctions_;
    //Using tbb::spin_mutex because it is able to be locked without the possibility of failing and throwing an exception.
    //This is needed, because FunctionHandle must be able to lock on the mutex within its destructor.
    //(In theory, the other mutexes could probably be reimplemented with that guarantee added, but working from
    //the specified interface, it is not safe to just use them for this).
    tbb::spin_mutex mutex_;
};
inline void swap(OperationInterrupter::FunctionHandle& l, OperationInterrupter::FunctionHandle& r) { l.swap(r); }
}//namespace hg
#endif //HG_OPERATION_interrupter_H
