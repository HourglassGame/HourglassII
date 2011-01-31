#ifndef HG_CONCURRENT_FRAME_UPDATE_SET_H
#define HG_CONCURRENT_FRAME_UPDATE_SET_H
#include <boost/thread/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include "FrameUpdateSet.h"
#include "BoostThreadHash.h"
namespace hg {

    class ConcurrentFrameUpdateSet {
    public:
        ConcurrentFrameUpdateSet();
        void add(const FrameUpdateSet& toAdd);
        FrameUpdateSet merge();
    private:
        typedef boost::unordered_map<boost::thread::id, FrameUpdateSet> MapType;
        MapType threadLocalMap_;
        boost::shared_mutex mutex_;
    };
}
#endif //HG_CONCURRENT_FRAME_UPDATE_SET_H
