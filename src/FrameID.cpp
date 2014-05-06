#include "FrameID.h"

#include "Frame.h"
#include "Universe.h"

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

#include <limits>
#include <cassert>

namespace hg {
FrameID::FrameID() noexcept :
        frame(-1),
        universeID(-1)
{
}
FrameID::FrameID(int frameNumber, UniverseID const &nuniverse) :
        frame(frameNumber),
        universeID(nuniverse)
{
    assert(isValidFrame());
}
//Creates a FrameID corresponding to the given Frame*
FrameID::FrameID(Frame const *toConvert) :
        frame(hg::getFrameNumber(toConvert)),
        universeID(getTimelineLength(hg::getUniverse(toConvert)))
{
}
FrameID FrameID::nextFrame(TimeDirection direction) const
{
    return
    isValidFrame() && nextFrameInSameUniverse(direction) ?
        FrameID(frame + direction, universeID) :
        FrameID();
}

bool FrameID::nextFrameInSameUniverse(TimeDirection direction) const
{
    return
        assert(isValidFrame()),
        (direction == REVERSE && frame != 0)
     || (direction == FORWARDS && frame != universeID.timelineLength - 1);
}
FrameID FrameID::arbitraryFrameInUniverse(int frameNumber) const
{
    return frameNumber >= 0 && frameNumber < universeID.timelineLength ?
        FrameID(frameNumber, universeID) : FrameID();
}

bool FrameID::operator==(FrameID const &o) const {
    return as_tie() == o.as_tie();
}

bool FrameID::operator<(FrameID const &o) const {
    return as_tie() < o.as_tie();
}
bool FrameID::isValidFrame() const {
    if (frame >= 0 && frame < universeID.timelineLength) {
        return true;
    }
    else {
        assert(frame == -1);
        assert(universeID.timelineLength == -1);
        return false;
    }
}
int FrameID::getFrameNumber() const {
    assert(isValidFrame());
    return frame;
}
UniverseID const &FrameID::getUniverse() const {
    return universeID;
}
std::size_t hash_value(FrameID const &toHash) {
    std::size_t seed(0);
    if (toHash.isValidFrame()) {
        boost::hash_combine(seed, toHash.getFrameNumber());
        boost::hash_combine(seed, toHash.getUniverse());
    }
    return seed;
}

bool isNullFrame(FrameID const &frame)
{
    return !frame.isValidFrame();
}
FrameID nextFrame(FrameID const &frame, TimeDirection direction)
{
    return frame.nextFrame(direction);
}
bool nextFrameInSameUniverse(FrameID const &frame, TimeDirection direction)
{
    return frame.nextFrameInSameUniverse(direction);
}
UniverseID getUniverse(FrameID const &frame)
{
    return frame.getUniverse();
}
int getFrameNumber(FrameID const &frame)
{
    return frame.getFrameNumber();
}

}//namespace hg
