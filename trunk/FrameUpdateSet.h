#ifndef HG_FRAME_UPDATE_SET_H
#define HG_FRAME_UPDATE_SET_H

#include <vector>
#include <boost/operators.hpp>
namespace hg {
class Frame;
//FrameUpdateSet is imlemented as a lazily sorted vector.
//The common use case is add, add, add, add ..., read, read, read, read...,
//So lazy sorting will probably be faster than an eagerly sorted system.

//... There was a long theoretical discussion here,
//but it was removed because it was irrelevant as always, 
//because actual performance testing may rule out any of the theory
//anyway. ...

//The overal conclusion was that both have equivalent complexity,
//but the constant factors in a vector are probably smaller,
//and the expensive operations are all put into one place in a vector.
//On the other hand the space complexity of a vector is much worse.
//The worst case space complexity of a sorted vector FrameUpdateSet is O(n^2),
//which occurs when every departure from every frame to every frame
//has changed.
//There is an upper limit to n given by the of the universe length.
//For a 10800 Frame universe this corresponds to a ~1GB FrameUpdateSet.
//These space complexities are not built into the data structure, but
//are implicit in the way that it is used.
//My measurements show that this is approximately 5x faster than
//a simple set implementation overall in typical high-propagation use.
//(I have not done conclusive tests by any means).
class FrameUpdateSet : boost::equality_comparable<FrameUpdateSet> {
public:
    FrameUpdateSet();

    FrameUpdateSet(FrameUpdateSet const& other);
    FrameUpdateSet& operator=(FrameUpdateSet const& other);
    void add(Frame* frame);
    void add(FrameUpdateSet const& other);
    void swap(FrameUpdateSet& other);

    typedef std::vector<Frame*> SetType;

    //Privacy leak here, it is an error to use the iterators as anything more than a BidirectionalIterator
    //I should define my own iterator class, but I can't be bothered right now
    typedef SetType::iterator iterator;
    typedef SetType::const_iterator const_iterator;
    typedef SetType::value_type value_type;
    typedef SetType::reference reference;
    typedef SetType::reference pointer;


    //Lazily made into set when these are called.
    //WARNING: although these are const, they may
    //modify the FrameUpdateSet if any non-const member
    //has been called since the previous call to make_set().
    //This means that if you are sharing a FrameUpdateSet then you must
    //ensure that it is a set, or ensure that all accesses are serialized
    //(even to these const members)
    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();
    std::size_t size() const;
    bool empty() const;
    //make_set() may be used to force the vector to be made into a set.
    //The only visible result of this is that it eliminates race conditions
    //in the lazy evaluation of the set.
    void make_set() const;
    bool operator==(FrameUpdateSet const& other);
private:
    mutable bool isSet_;
    mutable SetType updateSet_;
};
inline void swap(FrameUpdateSet& l, FrameUpdateSet& r) { l.swap(r); }

}
#endif //HG_FRAME_UPDATE_SET_H
