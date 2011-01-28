#include "Universe.h"
namespace hg {
//creates a top level universe
Universe::Universe(unsigned int timelineLength) :
initiatorFrame_(0),
frames_(timelineLength)
{
    assert(timelineLength > 0);
}
//returns initiatorFrame_
Frame* Universe::getInitiatorFrame() const
{
    return initiatorFrame_;
}
Frame* Universe::getEntryFrame(TimeDirection direction)
{
    switch (direction) {
        case FORWARDS:
            return &(*frames_.begin());
        case REVERSE:
            return &(*frames_.rbegin());
        default:
            assert(false);
    }
}
Frame* Universe::getArbitraryFrame(unsigned int frameNumber)
{
    if (frameNumber < frames_.size()) {
        return &frames_[frameNumber];
    }
    return 0;
}
//returns the length of this Universe's timeline
unsigned int Universe::getTimelineLength() const
{
    return frames_.size();
}
Universe::Universe(Frame* initiatorFrame, unsigned int timelineLength) :
initiatorFrame_(initiatorFrame),
frames_(timelineLength)
{
    assert(timelineLength > 0);
}
}