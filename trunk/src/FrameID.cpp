#include "FrameID.h"

#include "Frame.h"
#include "Universe.h"

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

#include <limits>
#include <cassert>

namespace hg {
FrameID::FrameID() :
        frame_(-1),
        universeID_(-1)
{
}
FrameID::FrameID(int frameNumber, const UniverseID& nuniverse) :
        frame_(frameNumber),
        universeID_(nuniverse)
{
    assert(isValidFrame());
}
//Creates a FrameID corresponding to the given Frame*
FrameID::FrameID(const Frame* toConvert) :
        frame_(toConvert->frameNumber_),
        universeID_(getTimelineLength(*toConvert->universe_))
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
FrameID FrameID::arbitraryFrameInUniverse(int frameNumber) const
{
    return frameNumber >= 0 && frameNumber < universeID_.timelineLength() ?
        FrameID(frameNumber, universeID_) : FrameID();
}

bool FrameID::operator==(const FrameID& o) const {
    return frame_ == o.frame_ && universeID_ == o.universeID_;
}

bool FrameID::operator<(const FrameID& o) const {
    return
    universeID_ == o.universeID_ ?
        frame_ < o.frame_ :
        universeID_ < o.universeID_;
}
bool FrameID::isValidFrame() const {
    return
    frame_ >= 0 && frame_ < universeID_.timelineLength() ?
        true :
        (assert(frame_ == -1),
        assert(universeID_.timelineLength() == -1),
        false);
}
int FrameID::getFrameNumber() const {
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

bool isNullFrame(FrameID const& frame)
{
    return !frame.isValidFrame();
}
FrameID nextFrame(FrameID const& frame, TimeDirection direction)
{
    return frame.nextFrame(direction);
}
bool nextFrameInSameUniverse(FrameID const& frame, TimeDirection direction)
{
    return frame.nextFrameInSameUniverse(direction);
}
UniverseID getUniverse(FrameID const& frame)
{
    return frame.getUniverse();
}
int getFrameNumber(FrameID const& frame)
{
    return frame.getFrameNumber();
}

}//namespace hg
