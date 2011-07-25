#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H

#include <vector>
#include <boost/operators.hpp>
namespace hg {
class Frame;
//At some stage consider making this into a lazily sorted vector.
//The common use case is add, add, add, add ..., read, read, read, read...,
//so it would probably be faster.
class FrameUpdateSet : boost::equality_comparable<FrameUpdateSet> {
public:
    FrameUpdateSet();

    FrameUpdateSet(FrameUpdateSet const& other);
    FrameUpdateSet& operator=(FrameUpdateSet const& other);
    void add(Frame* frame);
    void add(FrameUpdateSet const& other);
    void swap(FrameUpdateSet& other);

    void make_set();
    typedef std::vector<Frame*> SetType;

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
    bool operator==(FrameUpdateSet const& other);
private:
#ifndef NDEBUG
    bool isSet_;
#endif
    SetType updateSet_;
};
inline void swap(FrameUpdateSet& l, FrameUpdateSet& r) { l.swap(r); }

}
#endif //HG_FRAME_UPDATE_SET_H
