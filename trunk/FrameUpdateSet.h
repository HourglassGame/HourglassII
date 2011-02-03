#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H
#include "Frame.h"

#include <boost/unordered_set.hpp>
#include <set>
namespace hg {
    class FrameUpdateSet {
    public:
        FrameUpdateSet();
        
        void add(Frame* frame);
        void add(const FrameUpdateSet& other);
        void swap(FrameUpdateSet& other);
        
        typedef boost::unordered_set<Frame*> SetType;
        //typedef std::set<Frame*> SetType;
        
        //Privacy leak here, it is an error to use FrameUpdateSet::const_iterator as anything more than a ForwardIterator
        //I should define my own iterator class, but I can't be bothered right now
        typedef SetType::iterator iterator;
        typedef SetType::const_iterator const_iterator;
        typedef SetType::value_type value_type;
        typedef SetType::reference reference;
        typedef SetType::reference pointer;
        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
        size_t size() const;
        bool empty() const;
    private:
        SetType updateSet_;
    };
    bool operator==(const FrameUpdateSet& lhs, const FrameUpdateSet& rhs);
}
#endif //HG_FRAME_UPDATE_SET_H
