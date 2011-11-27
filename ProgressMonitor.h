#ifndef HG_PROGRESS_MONITOR_H
#define HG_PROGRESS_MONITOR_H
#include <tbb/task.h>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

namespace hg {
class ProgressMonitor {
public:
	void interrupt();
	void setInterruptionFunction(boost::function<void()>&& interruptionFunction);
private:
	bool interrupted_;
	boost::function<void()> interruptionFunction_;
	boost::mutex interruptionFunctionMutex_;
};
}//namespace hg
#endif /* HG_PROGRESS_MONITOR_H */
