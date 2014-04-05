#include "OperationInterrupter.h"
#include <boost/thread/locks.hpp>
#include "Foreach.h"
#include <thread>
namespace hg {
//Operation interrupter holds its lock while running the interruption function.
//This could lead to deadlock if the interruption function
//re-enters the OperationInterrupter. This seems somewhat unlikely, so it is probably OK.
//(Do not blindly change this to release the lock, the lock being held allows another
//*important* invariant to hold: "An interruption function will not be called after
//its associated FunctionHandle has been released.")
void OperationInterrupter::interrupt() {
	tbb::spin_mutex::scoped_lock lock(mutex_);
	interrupted_ = true;
    for (auto& f: interruptionFunctions_) {
        if (f) f();
        f = move_function<void()>();
    }
}

bool OperationInterrupter::interrupted() const {
    tbb::spin_mutex::scoped_lock lock(mutex_);
    return interrupted_;
}

OperationInterrupter::FunctionHandle OperationInterrupter::addInterruptionFunction(move_function<void()> interruptionFunction)
{
    assert(interruptionFunction
        && "There is no good reason to ever add an Interruption Function that can't be called");
    tbb::spin_mutex::scoped_lock lock(mutex_);
    if (interrupted_) {
        interruptionFunction();
        return FunctionHandle();
    }
    interruptionFunctions_.push_back(boost::move(interruptionFunction));
    return FunctionHandle(*this, interruptionFunctions_.end() - 1);
}
}//namespace hg
