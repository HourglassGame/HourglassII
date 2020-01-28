#ifndef HG_CONCURRENT_FRAME_UPDATE_SET_H
#define HG_CONCURRENT_FRAME_UPDATE_SET_H
#include "FrameUpdateSet.h"
#include <tbb/enumerable_thread_specific.h>
namespace hg {
class ConcurrentFrameUpdateSet final {
public:
    ConcurrentFrameUpdateSet();
    void add(FrameUpdateSet const &toAdd);
    void add(Frame *frame);
    FrameUpdateSet merge();
private:
    typedef tbb::enumerable_thread_specific<FrameUpdateSet> MapType;
    MapType threadLocalMap_;
};
}
#endif //HG_CONCURRENT_FRAME_UPDATE_SET_H
