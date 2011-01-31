#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H
#include "Frame.h"
#include <set>
namespace hg {
    class FrameUpdateSet {
    public:
        FrameUpdateSet() :
        updateSet_()
        {
        }
        
        void addFrame(Frame* frame) { 
            if (frame) {
                updateSet_.insert(frame);
            }
        }
        
        void add(const FrameUpdateSet& other) {
            updateSet_.insert(other.updateSet_.begin(), other.updateSet_.end());
        }
        void swap(FrameUpdateSet& other) {
            updateSet_.swap(other.updateSet_);
        }
        
        typedef ::std::set<Frame*> SetType;
        typedef SetType::const_iterator const_iterator;
        //Privacy leak here, it is an error to use FrameUpdateSet::const_iterator as anything more than a ForwardIterator
        //I should define my own iterator class, but I can't be bothered right now
        //typedef ::std::set<Frame*>::const_iterator const_iterator;
        typedef ::std::set<Frame*>::iterator iterator;
        typedef SetType::value_type value_type;
        const_iterator begin() const { return updateSet_.begin(); }
        const_iterator end() const { return updateSet_.end(); }
        iterator begin() { return updateSet_.begin(); }
        iterator end() { return updateSet_.end(); }
        size_t size() const { return updateSet_.size(); }
        bool empty() const { return updateSet_.empty(); }
    private:
        SetType updateSet_;
    };
}
#endif //HG_FRAME_UPDATE_SET_H
