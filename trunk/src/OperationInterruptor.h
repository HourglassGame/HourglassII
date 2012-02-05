#ifndef HG_OPERATION_INTERRUPTOR_H
#define HG_OPERATION_INTERRUPTOR_H
#include <tbb/spin_mutex.h>
#include "mt/boost/container/stable_vector.hpp"
#include "move_function.h"
#include <boost/swap.hpp>
#include <boost/move/move.hpp>

namespace hg {
class OperationInterruptor {
public:
    //Keeps the added interruption function
    class FunctionHandle {
    public:
        FunctionHandle() : interruptor_(), iterator_() {}
        FunctionHandle(
            OperationInterruptor& interruptor,
            mt::boost::container::stable_vector<move_function<void()> >::type::iterator iterator)
                : interruptor_(&interruptor), iterator_(iterator) {}
        
        FunctionHandle(BOOST_RV_REF(FunctionHandle) o) :
            interruptor_(), iterator_()
        {
            swap(o);
        }
        
        FunctionHandle& operator=(BOOST_RV_REF(FunctionHandle) o)
        {
            swap(o);
        }
        
        void swap(FunctionHandle& o) {
            boost::swap(interruptor_, o.interruptor_);
            boost::swap(iterator_, o.iterator_);
        }
        
        ~FunctionHandle() {
            if (interruptor_) {
                OperationInterruptor& i(*interruptor_);
                tbb::spin_mutex::scoped_lock lock(i.mutex_);
                i.interruptionFunctions_.erase(iterator_);
            }
        }
    private:
        OperationInterruptor* interruptor_;
        mt::boost::container::stable_vector<move_function<void()> >::type::iterator iterator_;
        BOOST_MOVABLE_BUT_NOT_COPYABLE(FunctionHandle)
    };
    
    //Calls all currently active interruptionFunctions
    //Causes attempts to add new interruptionFunctions to immediately call
    //the passed interruptionFunction.
	void interrupt();
    
    //While the returned FunctionHandle (or a FunctionHandle moved from the returned FunctionHandle) is alive,
    //a call to `interrupt` will call interruptionFunction. The returned FunctionHandle must not outlive
    //the OperationInterruptor.
	FunctionHandle addInterruptionFunction(move_function<void()> interruptionFunction);

private:
	bool interrupted_;
	mt::boost::container::stable_vector<move_function<void()> >::type interruptionFunctions_;
    //Using tbb::spin_mutex because it is able to be locked without the possibility of failing and throwing and exception.
    //This is needed, because FunctionHandle must be able to lock on the mutex within its destructor.
    //(In theory, the other mutexes could probably be reimplemented with that guarantee added, but working from
    //the specified interface, it is not safe to)
    tbb::spin_mutex mutex_;
};
}//namespace hg
#endif //HG_OPERATION_INTERRUPTOR_H
