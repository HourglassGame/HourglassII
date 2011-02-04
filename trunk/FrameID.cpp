#include "FrameID.h"

#include <boost/functional/hash.hpp>

#include <limits>
#include <cassert>

namespace hg {

FrameID::FrameID() :
frame_(::std::numeric_limits<size_t>::max()),
universeID_(0)
{
}

FrameID::FrameID(size_t time, const UniverseID& nuniverse) :
frame_(time),
universeID_(nuniverse)
{
    assert(isValidFrame());
}
#if 0
//Creates a FrameID corresponding to the given Frame*
FrameID::FrameID(const Frame* toConvert) :
frame_(toConvert->frameNumber_),
universeID_()
{
    std::vector<SubUniverse>& nestTrain;
    const Universe* universe(&(toConvert->universe_));
    for(; universe->initiatorFrame_; universe = &(universe->initiatorFrame_->universe_))
    {
        universeID_.nestTrain.push_back(SubUniverse(universe->initiatorFrame_->frameNumber_, *(universe->initiatorID_)));
    }
    boost::reverse(universe_.nestTrain);
    universeID_.timelineLength_ = universe->frames_.size();
}
#endif
FrameID FrameID::nextFrame(TimeDirection direction) const
{
    if (!isValidFrame()) {
        return FrameID();
    }
    else if (nextFramePauseLevelDifference(direction) == 0) {
        return FrameID(frame_ + direction, universeID_);
    }
    else {
        return universeID_.parentFrame().nextFrame(direction);
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
                (direction == FORWARDS && frame_ == universeID_.timelineLength()))) {
        return depthAccumulator;
    }
    else {
        return universeID_.parentFrame().nextFramePauseLevelDifferenceAux(depthAccumulator + 1, direction);
    }
}

FrameID FrameID::arbitraryFrameInUniverse(size_t frameNumber) const
{
    if (frameNumber >= universeID_.timelineLength()) {
        return FrameID();
    }
    else {
        return FrameID(frameNumber, universeID_);
    }
}

FrameID FrameID::parentFrame() const
{
    return universeID_.parentFrame();
}

FrameID FrameID::arbitraryChildFrame(const PauseInitiatorID& initatorID,
                                           size_t frameNumber) const
{
    return FrameID(frameNumber, universeID_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

FrameID FrameID::entryChildFrame(const PauseInitiatorID& initatorID,
                                       TimeDirection direction) const
{
    assert(initatorID.timelineLength_ != 0);
    return FrameID(direction == FORWARDS ? 0 : initatorID.timelineLength_ - 1,
                      universeID_.getSubUniverse(SubUniverse(frame_, initatorID)));
}

bool FrameID::operator==(const FrameID& other) const
{
    return frame_ == other.frame_ && universeID_ == other.universeID_;
}

bool FrameID::operator!=(const FrameID& other) const
{
    return !(*this==other);
}

bool FrameID::operator<(const FrameID& other) const
{
    if (universeID_ == other.universeID_) {
        return frame_ < other.frame_;
    }
    else {
        return universeID_ < other.universeID_;
    }

}

bool FrameID::isValidFrame() const
{
    if (universeID_.timelineLength() < frame_) {
        assert(frame_ == ::std::numeric_limits<size_t>::max());
        assert(universeID_.timelineLength() == 0);
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
    ::boost::hash_combine(seed, toHash.universeID_);
    return seed;
}
}
