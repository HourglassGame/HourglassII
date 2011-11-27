#include "ProgressMonitor.h"
#include <boost/thread/locks.hpp>
namespace hg {
void ProgressMonitor::interrupt() {
	boost::lock_guard<boost::mutex> lock(interruptionFunctionMutex_);
	interrupted_ = true;
	if (interruptionFunction_) {
		interruptionFunction_();
	}
}

void ProgressMonitor::setInterruptionFunction(boost::function<void()>&& interruptionFunction)
{
	boost::lock_guard<boost::mutex> lock(interruptionFunctionMutex_);
	interruptionFunction_.swap(interruptionFunction);
	if (interrupted_ && interruptionFunction_) {
		interruptionFunction_();
	}
}
}

