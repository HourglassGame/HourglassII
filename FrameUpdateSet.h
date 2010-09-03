#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H
#include <boost/static_assert.hpp>
#include "FrameID.h"
#include <set>
namespace hg {
    class FrameUpdateSet {
    public:
        void addFrame(FrameID frame);
        
        void add(const FrameUpdateSet& other);
        
        //Privacy leak here, it is an error to use const_iterator as anything more than a forward_iterator
        //I should define my own iterator class, but I can't be bothered right now
        typedef ::std::set<FrameID>::const_iterator const_iterator;
        
        const_iterator begin() const;
        const_iterator end() const;
    private:
        ::std::set<FrameID> updateSet;
    };
}
#endif //HG_FRAME_UPDATE_SET_H
