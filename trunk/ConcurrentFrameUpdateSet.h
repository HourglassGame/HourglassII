#ifndef HG_CONCURRENT_FRAME_UPDATE_SET_H
#define HG_CONCURRENT_FRAME_UPDATE_SET_H
#include <boost/thread/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include "FrameUpdateSet.h"
#include "BoostThreadHash.h"
#include "BoostHashCompare.h"
#include <tbb/compat/thread>
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
