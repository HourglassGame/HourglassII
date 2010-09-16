#include "SimpleFrameID.h"
#include <limits>
using namespace ::hg;
SimpleFrameID::SimpleFrameID() :
frame_(::std::numeric_limits<unsigned int>::max()),
timelineLength_(0)
{
}

SimpleFrameID::SimpleFrameID(unsigned int time, unsigned int timelineLength) :
frame_(time),
timelineLength_(timelineLength)
{
    assert(isValidFrame());
}

// returns the normal next frame for things moving in direction TimeDirection
SimpleFrameID SimpleFrameID::nextFrame(TimeDirection direction) const
{
    if (nextFrameInUniverse(direction)) {
        return SimpleFrameID(frame_ + direction, timelineLength_);
    }
    else {
        return SimpleFrameID();
    }
}

bool SimpleFrameID::nextFrameInUniverse(TimeDirection direction) const
{
    return (frame_ + direction) < timelineLength_;
}

SimpleFrameID SimpleFrameID::arbitraryFrameInUniverse(unsigned int frameNumber) const
{   
    if (frameNumber < timelineLength_) {
        return SimpleFrameID(frameNumber, timelineLength_);
    }
    else {
        return SimpleFrameID();
    }
}

bool SimpleFrameID::operator==(const SimpleFrameID& other) const
{
    return frame_ == other.frame_ && timelineLength_ == other.timelineLength_;
}

bool SimpleFrameID::operator<(const SimpleFrameID& other) const
{
    if (timelineLength_ == other.timelineLength_) {
        return frame_ < other.frame_;
    }
    else {
        return timelineLength_ < other.timelineLength_;
    }
}

bool SimpleFrameID::isValidFrame() const
{
    if (frame_ > timelineLength_) {
        assert(timelineLength_ == 0);
        assert(frame_ == ::std::numeric_limits<unsigned int>::max());
        return false;
    }
    return true;
}

::std::size_t hg::hash_value(const SimpleFrameID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.frame_);
    ::boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
