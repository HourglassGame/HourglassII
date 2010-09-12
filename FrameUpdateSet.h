#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H
#include "SimpleFrameID.h"
#include <set>
namespace hg {
    class FrameUpdateSet {
    public:
        FrameUpdateSet() :
        updateSet_()
        {
        }
        
        void addFrame(SimpleFrameID frame) { 
            if (frame.isValidFrame()) {
                updateSet_.insert(frame);
            }
        }
        
        void add(const FrameUpdateSet& other) { 
            updateSet_.insert(other.updateSet_.begin(), other.updateSet_.end());
        }
        
        //Privacy leak here, it is an error to use FrameUpdateSet::const_iterator as anything more than a forward_iterator
        //I should define my own iterator class, but I can't be bothered right now
        typedef ::std::set<SimpleFrameID>::const_iterator const_iterator;
        
        const_iterator begin() const { return updateSet_.begin(); }
        const_iterator end() const { return updateSet_.end(); }
        size_t size() const { return updateSet_.size(); }
    private:
        ::std::set<SimpleFrameID> updateSet_;
    };
}
#endif //HG_FRAME_UPDATE_SET_H
