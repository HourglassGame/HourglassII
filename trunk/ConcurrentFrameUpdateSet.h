#ifndef HG_CONCURRENT_FRAME_UPDATE_SET_H
#define HG_CONCURRENT_FRAME_UPDATE_SET_H
#include "FrameUpdateSet.h"
#include <tbb/enumerable_thread_specific.h>
namespace hg {
    class ConcurrentFrameUpdateSet {
    public:
        ConcurrentFrameUpdateSet();
        void add(const FrameUpdateSet& toAdd);
        FrameUpdateSet merge();
    private:
        tbb::enumerable_thread_specific<FrameUpdateSet> threadLocalMap_;
    };
}
#endif //HG_CONCURRENT_FRAME_UPDATE_SET_H
