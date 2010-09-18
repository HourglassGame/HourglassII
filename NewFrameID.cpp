#include "NewFrameID.h"
#include <limits>
#include <cassert>

using namespace ::hg;

NewFrameID::NewFrameID() :
frame_(::std::numeric_limits<unsigned int>::max()),
universe_(0)
{
}

NewFrameID::NewFrameID(unsigned int time, const UniverseID& universe) :
frame_(time),
universe_(universe)
{
    assert(isValidFrame());
}

NewFrameID NewFrameID::nextFrame(TimeDirection direction) const
{
    if (!isValidFrame()) {
        return NewFrameID();
    }
    else if (nextFrameInUniverse(direction)) {
        return NewFrameID(frame_ + direction, universe_);
    }
    else {
        return universe_.parentFrame().nextFrame(direction);
    }
}

bool NewFrameID::nextFrameInUniverse(TimeDirection direction) const
{
    assert(isValidFrame());
    return !((direction == REVERSE && frame_ == 0)
             || (direction == FORWARDS && frame_ == universe_.timelineLength()));
}

NewFrameID NewFrameID::arbitraryFrameInUniverse(unsigned int frameNumber) const
{
    if (frameNumber >= universe_.timelineLength()) {
        return NewFrameID();
    }
    else {
        return NewFrameID(frameNumber, universe_);
    }
}

NewFrameID NewFrameID::parentFrame() const
{
    return universe_.parentFrame();
}

NewFrameID NewFrameID::arbitraryChildFrame(const PauseInitiatorID& initatorID,
                                           unsigned int frameNumber) const
{
    return NewFrameID(frameNumber, universe_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

NewFrameID NewFrameID::entryChildFrame(const PauseInitiatorID& initatorID,
                                       unsigned int pauseLength,
                                       TimeDirection direction) const
{
    assert(initatorID.timelineLength_ != 0);
    return NewFrameID(direction == FORWARDS ? 0 : initatorID.timelineLength_ - 1,
                      universe_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

bool NewFrameID::operator==(const NewFrameID& other) const
{
    return frame_ == other.frame_ && universe_ == other.universe_;
}

bool NewFrameID::operator<(const NewFrameID& other) const
{
    if (universe_ == other.universe_) {
        return frame_ < other.frame_;
    }
    else {
        return universe_ < other.universe_;
    }

}

bool NewFrameID::isValidFrame() const
{
    if (universe_.timelineLength() < frame_) {
        assert(frame_ == ::std::numeric_limits<unsigned int>::max());
        assert(universe_.timelineLength() == 0);
        return false;
    }
    return true;
}

::std::size_t hg::hash_value(const NewFrameID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.frame_);
    ::boost::hash_combine(seed, toHash.universe_);
    return seed;
}
