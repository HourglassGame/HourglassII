#include "Universe.h"
#include "FrameID.h"

#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include "Foreach.h"
#include <boost/swap.hpp>

#include <functional>
#include <cassert>

namespace hg {
//Updates the universe_ pointers in frames_
void Universe::fixFramesUniverses()
{
    foreach (Frame& frame, frames_) {
        frame.correctUniverse(*this);
    }
}

void Universe::swap(Universe& other)
{
    boost::swap(frames_, other.frames_);
    fixFramesUniverses();
    other.fixFramesUniverses();
}

//creates a top level universe
Universe::Universe(std::size_t timelineLength) :
        frames_()
{
    assert(timelineLength > 0);
    frames_.reserve(timelineLength);
    foreach (std::size_t i, boost::irange<std::size_t>(0, timelineLength)) {
        frames_.push_back(Frame(i, *this));
    }
}
Frame* Universe::getEntryFrame(TimeDirection direction)
{
    assert(!frames_.empty());
    switch (direction) {
    case FORWARDS:
        return &(*frames_.begin());
    case REVERSE:
        return &(*frames_.rbegin());
    default:
        assert(false);
    }
    //Never reached
    return 0;
}
Frame* Universe::getArbitraryFrame(std::size_t frameNumber)
{
    assert(!frames_.empty());
    return frameNumber < frames_.size() ? &frames_[frameNumber] : 0;
}
//returns the length of this Universe's timeline
std::size_t Universe::getTimelineLength() const
{
    assert(!frames_.empty());
    return frames_.size();
}

//Returns the first frame in the universe for objects travelling
//in TimeDirection direction.
Frame* getEntryFrame(Universe& universe, TimeDirection direction)
{
    return universe.getEntryFrame(direction);
}
//Returns the frame with the index frameNumber within the universe, 
//or the NullFrame if no such frame exists
Frame* getArbitraryFrame(Universe& universe,std::size_t frameNumber)
{
    return universe.getArbitraryFrame(frameNumber);
}
//returns the length of this Universe's timeline
std::size_t getTimelineLength(Universe const& universe) {
    return universe.getTimelineLength();
}
Frame* Universe::getFrame(const FrameID& whichFrame)
{
    assert(getTimelineLength() == whichFrame.getUniverse().timelineLength());
    return getArbitraryFrame(whichFrame.getFrameNumber());
}
}
