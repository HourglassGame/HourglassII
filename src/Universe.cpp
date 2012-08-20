#include "Universe.h"
#include "FrameID.h"

#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/swap.hpp>

#include "Foreach.h"

#include <functional>
#include <cassert>

namespace hg {
Universe::Universe(BOOST_RV_REF(Universe) o) :
	frames_(boost::move(o.frames_))
{
	assert(!frames_.empty());
	fixFramesUniverses();
}
Universe& Universe::operator=(BOOST_RV_REF(Universe) o)
{
	assert(!o.frames_.empty());
	frames_ = boost::move(o.frames_);
	fixFramesUniverses();
	return *this;
}

//Updates the universe_ pointers in frames_
void Universe::fixFramesUniverses()
{
    foreach (Frame& frame, frames_) {
        frame.correctUniverse(*this);
    }
}

//creates a top level universe
Universe::Universe(int timelineLength) :
    frames_()
{
    assert(timelineLength > 0);
    frames_.reserve(timelineLength);
    foreach (int i, boost::irange<int>(0, timelineLength)) {
        frames_.push_back(Frame(i, *this));
    }
	assert(!frames_.empty());
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
Frame* Universe::getArbitraryFrame(int frameNumber)
{
    assert(!frames_.empty());
    return frameNumber >= 0 && frameNumber < getTimelineLength() ? &frames_[frameNumber] : 0;
}
Frame* Universe::getArbitraryFrameClamped(int frameNumber)
{
    assert(!frames_.empty());
    if (frameNumber < 0) return &(*frames_.begin());
    if (frameNumber >= getTimelineLength()) return &(*frames_.rbegin());
    return &frames_[frameNumber];
}
//returns the length of this Universe's timeline
int Universe::getTimelineLength() const
{
    assert(!frames_.empty());
    return static_cast<int>(frames_.size());
}

//Returns the first frame in the universe for objects travelling
//in TimeDirection direction.
Frame* getEntryFrame(Universe& universe, TimeDirection direction)
{
    return universe.getEntryFrame(direction);
}
//Returns the frame with the index frameNumber within the universe,
//or the NullFrame if no such frame exists
Frame* getArbitraryFrame(Universe& universe, int frameNumber)
{
    return universe.getArbitraryFrame(frameNumber);
}
//Returns the frame with the index closest to frameNumber within the universe.
Frame* getArbitraryFrameClamped(Universe& universe, int frameNumber)
{
    return universe.getArbitraryFrameClamped(frameNumber);
}
//returns the length of this Universe's timeline
int getTimelineLength(Universe const& universe)
{
    return universe.getTimelineLength();
}
Frame* Universe::getFrame(FrameID const& whichFrame)
{
    assert(getTimelineLength() == whichFrame.getUniverse().timelineLength());
    return getArbitraryFrame(whichFrame.getFrameNumber());
}
}//namespace hg
