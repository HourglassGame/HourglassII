#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H

#include <boost/unordered_set.hpp>
#include <boost/move/move.hpp>
namespace hg {
class Frame;
//At some stage consider making this into a lazily sorted vector.
//The common use case is add, add, add, add ..., read, read, read, read...,
//so it would probably be faster.
class FrameUpdateSet {
public:
    FrameUpdateSet();
    
    FrameUpdateSet(FrameUpdateSet const& other);
    FrameUpdateSet& operator=(BOOST_COPY_ASSIGN_REF(FrameUpdateSet) other);
    FrameUpdateSet(BOOST_RV_REF(FrameUpdateSet) other);
    FrameUpdateSet& operator=(BOOST_RV_REF(FrameUpdateSet) other);

    void add(Frame* frame);
    void add(FrameUpdateSet const& other);
    void swap(FrameUpdateSet& other);

    typedef boost::unordered_set<Frame*> SetType;

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
    std::size_t size() const;
    bool empty() const;
private:
    SetType updateSet_;
    BOOST_COPYABLE_AND_MOVABLE(FrameUpdateSet)
};
bool operator==(FrameUpdateSet const& lhs, FrameUpdateSet const& rhs);
}
#endif //HG_FRAME_UPDATE_SET_H
