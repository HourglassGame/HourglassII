#include "FrameID.h"

#include <boost/functional/hash.hpp>

#include <limits>
#include <cassert>

namespace hg {

FrameID::FrameID() :
frame_(::std::numeric_limits<unsigned int>::max()),
universe_(0)
{
}

FrameID::FrameID(unsigned int time, const UniverseID& nuniverse) :
frame_(time),
universe_(nuniverse)
{
    assert(isValidFrame());
}

FrameID FrameID::nextFrame(TimeDirection direction) const
{
    if (!isValidFrame()) {
        return FrameID();
    }
    else if (nextFramePauseLevelDifference(direction) == 0) {
        return FrameID(frame_ + direction, universe_);
    }
    else {
        return universe_.parentFrame().nextFrame(direction);
    }
}

unsigned int FrameID::nextFramePauseLevelDifference(TimeDirection direction) const
{
    assert(isValidFrame());
    return nextFramePauseLevelDifferenceAux(0, direction);

}
//Probably won't be recursive in final version, but it seemed the easiest way.
unsigned int FrameID::nextFramePauseLevelDifferenceAux(unsigned int depthAccumulator, TimeDirection direction) const
{
    if (        !isValidFrame()
        ||
                !((direction == REVERSE && frame_ == 0)
            ||
                (direction == FORWARDS && frame_ == universe_.timelineLength()))) {
        return depthAccumulator;
    }
    else {
        return universe_.parentFrame().nextFramePauseLevelDifferenceAux(depthAccumulator + 1, direction);
    }
}

FrameID FrameID::arbitraryFrameInUniverse(unsigned int frameNumber) const
{
    if (frameNumber >= universe_.timelineLength()) {
        return FrameID();
    }
    else {
        return FrameID(frameNumber, universe_);
    }
}

FrameID FrameID::parentFrame() const
{
    return universe_.parentFrame();
}

FrameID FrameID::arbitraryChildFrame(const PauseInitiatorID& initatorID,
                                           unsigned int frameNumber) const
{
    return FrameID(frameNumber, universe_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

FrameID FrameID::entryChildFrame(const PauseInitiatorID& initatorID,
                                       TimeDirection direction) const
{
    assert(initatorID.timelineLength_ != 0);
    return FrameID(direction == FORWARDS ? 0 : initatorID.timelineLength_ - 1,
                      universe_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

bool FrameID::operator==(const FrameID& other) const
{
    return frame_ == other.frame_ && universe_ == other.universe_;
}

bool FrameID::operator!=(const FrameID& other) const
{
    return !(*this==other);
}

bool FrameID::operator<(const FrameID& other) const
{
    if (universe_ == other.universe_) {
        return frame_ < other.frame_;
    }
    else {
        return universe_ < other.universe_;
    }

}

bool FrameID::isValidFrame() const
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

::std::size_t hash_value(const FrameID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.frame_);
    ::boost::hash_combine(seed, toHash.universe_);
    return seed;
}
}
