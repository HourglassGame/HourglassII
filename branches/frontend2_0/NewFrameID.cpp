#include "NewFrameID.h"

#include <boost/functional/hash.hpp>

#include <limits>
#include <cassert>

namespace hg {

NewFrameID::NewFrameID() :
frame_(::std::numeric_limits<unsigned int>::max()),
universe_(0)
{
}

NewFrameID::NewFrameID(unsigned int time, const UniverseID& nuniverse) :
frame_(time),
universe_(nuniverse)
{
    assert(isValidFrame());
}

NewFrameID NewFrameID::nextFrame(TimeDirection direction) const
{
    if (!isValidFrame()) {
        return NewFrameID();
    }
    else if (nextFrameInUniverse(direction) == 0) {
        return NewFrameID(frame_ + direction, universe_);
    }
    else {
        return universe_.parentFrame().nextFrame(direction);
    }
}

unsigned int NewFrameID::nextFrameInUniverse(TimeDirection direction) const
{
    assert(isValidFrame());
    return nextFrameInUniverseAux(0, direction);

}
//Probably won't be recursive in final version, but it seemed the easiest way.
unsigned int NewFrameID::nextFrameInUniverseAux(unsigned int depthAccumulator, TimeDirection direction) const
{
    if (        !isValidFrame()
        ||
                !((direction == REVERSE && frame_ == 0)
            ||
                (direction == FORWARDS && frame_ == universe_.timelineLength()))) {
        return depthAccumulator;
    }
    else {
        return universe_.parentFrame().nextFrameInUniverseAux(depthAccumulator + 1, direction);
    }
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

bool NewFrameID::operator!=(const NewFrameID& other) const
{
    return !(*this==other);
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
    else {
        return true;
    }
}

::std::size_t hash_value(const NewFrameID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.frame_);
    ::boost::hash_combine(seed, toHash.universe_);
    return seed;
}
}
