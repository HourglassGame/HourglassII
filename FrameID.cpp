#include "FrameID.h"

#include "Frame.h"
#include "Universe.h"

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

#include <limits>
#include <cassert>

namespace hg {
FrameID::FrameID() :
        frame_(std::numeric_limits<std::size_t>::max()),
        universeID_(0)
{
}
FrameID::FrameID(std::size_t time, const UniverseID& nuniverse) :
        frame_(time),
        universeID_(nuniverse)
{
    assert(isValidFrame());
}
//Creates a FrameID corresponding to the given Frame*
FrameID::FrameID(const Frame* toConvert) :
        frame_(toConvert->frameNumber_),
        universeID_(getTimelineLength(toConvert->universe_))
{
}
FrameID FrameID::nextFrame(TimeDirection direction) const
{
    return 
    isValidFrame() && nextFrameInSameUniverse(direction) ? 
        FrameID(frame_ + direction, universeID_) :
        FrameID();
}

bool FrameID::nextFrameInSameUniverse(TimeDirection direction) const
{
    return
        assert(isValidFrame()),
        (direction == REVERSE && frame_ != 0)
     || (direction == FORWARDS && frame_ != universeID_.timelineLength() - 1);
}
FrameID FrameID::arbitraryFrameInUniverse(std::size_t frameNumber) const
{
    return frameNumber < universeID_.timelineLength() ?
        FrameID(frameNumber, universeID_) : FrameID();
}

bool FrameID::operator==(const FrameID& other) const {
    return frame_ == other.frame_ && universeID_ == other.universeID_;
}

bool FrameID::operator<(const FrameID& other) const {
    return
    universeID_ == other.universeID_ ?
        frame_ < other.frame_ :
        universeID_ < other.universeID_;
}
bool FrameID::isValidFrame() const {
    return
    frame_ < universeID_.timelineLength() ?
        true :
        (assert(frame_ == std::numeric_limits<std::size_t>::max()),
        assert(universeID_.timelineLength() == 0),
        false);
}
std::size_t FrameID::getFrameNumber() const {
    assert (isValidFrame());
    return frame_;
}
UniverseID const& FrameID::getUniverse() const {
    return universeID_;
}
std::size_t hash_value(const FrameID& toHash) {
    std::size_t seed(0);
    return
    toHash.isValidFrame() ?
        seed :
        (boost::hash_combine(seed, toHash.getFrameNumber()),
        boost::hash_combine(seed, toHash.getUniverse()),
        seed);
}
}//namespace hg
