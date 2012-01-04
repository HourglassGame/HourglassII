#ifndef HG_OPERATION_INTERRUPTOR_H
#define HG_OPERATION_INTERRUPTOR_H
#include <tbb/atomic.h>
namespace hg {
    class OperationInterruptor {
    public:
        OperationInterruptor(bool interrupted = false) :
            interrupted_()
        {
            //Assigned, rather than initialised, because a
            //tbb::atomic<T> cannot be constructed from a
            //T.
            interrupted_ = interrupted;
        }
        bool interrupted() const { return interrupted_; }
        void interrupt() { interrupted_ = true; }
    private:
        tbb::atomic<bool> interrupted_;
    };
    extern OperationInterruptor const g_nullInterruptor;
}
#endif //HG_OPERATION_INTERRUPTOR_H
