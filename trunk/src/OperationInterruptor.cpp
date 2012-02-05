#include "OperationInterruptor.h"
#include <boost/thread/locks.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
namespace hg {
//Operation interruptor holds its lock while running the interruption function.
//This could potentially lead to deadlock, if the interruption function
//re-enters the OperationInterruptor. This seems somewhat unlikely, so it is probably OK.
//(Do not blindly change this to release the lock, the lock being held allows another
//*important* invariant to hold: "An interruption function will not be called after
//its associated FunctionHandle has been released.")
void OperationInterruptor::interrupt() {
	tbb::spin_mutex::scoped_lock lock(mutex_);
	interrupted_ = true;
    foreach (move_function<void()>& f, interruptionFunctions_) {
        f();
        f = move_function<void()>();
    }
}

OperationInterruptor::FunctionHandle OperationInterruptor::addInterruptionFunction(move_function<void()> interruptionFunction)
{
	tbb::spin_mutex::scoped_lock lock(mutex_);
    interruptionFunctions_.push_back(boost::move(interruptionFunction));
    if (interrupted_ && interruptionFunctions_.back()) {
		interruptionFunctions_.back()();
        interruptionFunctions_.back() = move_function<void()>();
	}
    return FunctionHandle(*this, interruptionFunctions_.end() - 1);
}
}//namespace hg
