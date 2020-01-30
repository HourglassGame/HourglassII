#include "Universe.h"
#include "FrameID.h"
#include "Frame.h"

#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/swap.hpp>

#include <functional>
#include <cassert>
#include <utility>

namespace hg {
Universe::Universe(int timelineLength, unsigned defaultSpeedOfTime) :
    frames()
{
    assert(timelineLength > 0);
    frames.reserve(timelineLength);
    for (int i: boost::irange<int>(0, timelineLength)) {
        frames.push_back(Frame(i, defaultSpeedOfTime, hg::PARADOX_PRESSURE_PER_FRAME,  *this));
    }
    assert(!frames.empty());
}

Universe::Universe(Universe const &o) :
    frames(o.frames)
{
    fixFramesEverything();
}
Universe &Universe::operator=(Universe const &o)
{
    frames = o.frames;
    fixFramesEverything();
    return *this;
}

Universe::Universe(Universe &&o) noexcept :
    frames(std::move(o.frames))
{
    assert(!frames.empty());
    fixFramesUniverses();
}
Universe &Universe::operator=(Universe &&o) noexcept
{
    assert(!o.frames.empty());
    frames = std::move(o.frames);
    fixFramesUniverses();
    return *this;
}

//updates the universe and Frame pointers in `frames`
void Universe::fixFramesEverything() {
    fixFramesUniverses();
    for (Frame &frame: frames) {
        frame.correctDepartureFramePointers(Universe_Frame_access{}, FramePointerUpdater(*this));
    }
    for (Frame &frame: frames) {
        frame.correctArrivalFramePointers(Universe_Frame_access{}, FramePointerUpdater(*this));
    }
    for (Frame &frame: frames) {
        frame.correctArrivalObjectListPointers(Universe_Frame_access{});
    }
}

//Updates the `universe` pointers in `frames`
void Universe::fixFramesUniverses() noexcept
{
    for (Frame &frame: frames) {
        frame.correctUniverse(Universe_Frame_access{},*this);
    }
}


template<typename UniverseT>
Universe::FrameMatchingUniverseConstness<UniverseT> *Universe::getFrameImpl(UniverseT &universe, FrameID const &whichFrame) {
    assert(hg::getTimelineLength(universe) == hg::getTimelineLength(whichFrame.getUniverse()));
    return universe.getArbitraryFrame(whichFrame.getFrameNumber());
}
Frame *Universe::getFrame(FrameID const &whichFrame)
{
    return getFrameImpl(*this, whichFrame);
}
Frame const *Universe::getFrame(FrameID const &whichFrame) const
{
    return getFrameImpl(*this, whichFrame);
}

template<typename UniverseT>
Universe::FrameMatchingUniverseConstness<UniverseT> *Universe::getEntryFrameImpl(UniverseT &universe, TimeDirection direction) {
    assert(!universe.frames.empty());
    switch (direction) {
    case TimeDirection::FORWARDS:
        return &(*universe.frames.begin());
    case TimeDirection::REVERSE:
        return &(*universe.frames.rbegin());
    default:
        assert(false);
    }
    //Never reached
    return nullptr;
}

Frame *Universe::getEntryFrame(TimeDirection direction)
{
    return getEntryFrameImpl(*this, direction);
}
Frame const *Universe::getEntryFrame(TimeDirection direction) const
{
    return getEntryFrameImpl(*this, direction);
}


template<typename UniverseT>
Universe::FrameMatchingUniverseConstness<UniverseT> *Universe::getArbitraryFrameImpl(UniverseT &universe, int frameNumber)
{
    assert(!universe.frames.empty());
    return frameNumber >= 0 && frameNumber < universe.getTimelineLength() ? &universe.frames[frameNumber] : nullptr;
}
Frame *Universe::getArbitraryFrame(int frameNumber)
{
    return getArbitraryFrameImpl(*this, frameNumber);
}
Frame const *Universe::getArbitraryFrame(int frameNumber) const
{
    return getArbitraryFrameImpl(*this, frameNumber);
}

template<typename UniverseT>
Universe::FrameMatchingUniverseConstness<UniverseT> *Universe::getArbitraryFrameClampedImpl(UniverseT &universe, int frameNumber) {
    assert(!universe.frames.empty());
    if (frameNumber < 0) return &(*universe.frames.begin());
    if (frameNumber >= universe.getTimelineLength()) return &(*universe.frames.rbegin());
    return &universe.frames[frameNumber];
}
Frame *Universe::getArbitraryFrameClamped(int frameNumber)
{
    return Universe::getArbitraryFrameClampedImpl(*this, frameNumber);
}
Frame const *Universe::getArbitraryFrameClamped(int frameNumber) const
{
    return Universe::getArbitraryFrameClampedImpl(*this, frameNumber);
}

//Returns the length of this Universe's timeline
int Universe::getTimelineLength() const
{
    assert(!frames.empty() &&
            frames.size() <= static_cast<decltype(frames.size())>(std::numeric_limits<int>::max()));
    return static_cast<int>(frames.size());
}

//Returns the first frame in the universe for objects travelling
//in TimeDirection direction.
Frame *getEntryFrame(Universe &universe, TimeDirection direction)
{
    return universe.getEntryFrame(direction);
}
Frame const *getEntryFrame(Universe const &universe, TimeDirection direction)
{
    return universe.getEntryFrame(direction);
}

//Returns the frame with the index frameNumber within the universe,
//or the NullFrame if no such frame exists
Frame *getArbitraryFrame(Universe &universe, int frameNumber)
{
    return universe.getArbitraryFrame(frameNumber);
}
Frame const *getArbitraryFrame(Universe const &universe, int frameNumber)
{
    return universe.getArbitraryFrame(frameNumber);
}

//Returns the frame with the index closest to frameNumber within the universe.
Frame *getArbitraryFrameClamped(Universe &universe, int frameNumber)
{
    return universe.getArbitraryFrameClamped(frameNumber);
}
Frame const *getArbitraryFrameClamped(Universe const &universe, int frameNumber)
{
    return universe.getArbitraryFrameClamped(frameNumber);
}

//Returns the length of this Universe's timeline
int getTimelineLength(Universe const &universe)
{
    return universe.getTimelineLength();
}

}//namespace hg
