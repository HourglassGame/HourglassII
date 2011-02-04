#include "Universe.h"
#include "FrameID.h"
namespace hg {
Universe::Universe() :
initiatorFrame_(0),
frames_(), 
initiatorID_(0)
{
}
Universe::Universe(const Universe& other) :
initiatorFrame_(0),
frames_(),
initiatorID_(0)
{
    //Ok to copy construct as long as there are no frames which know where the universe is 
    //and there are no frames which know where the frames in this universe are
    assert(other.frames_.empty());
}
//creates a top level universe
Universe::Universe(size_t timelineLength) :
initiatorFrame_(0),
frames_(),
initiatorID_(0)
{
    assert(timelineLength > 0);
    construct(initiatorFrame_, timelineLength, *initiatorID_);
}
//returns initiatorFrame_
Frame* Universe::getInitiatorFrame() const
{
    assert(!frames_.empty());
    return initiatorFrame_;
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
}
Frame* Universe::getArbitraryFrame(size_t frameNumber)
{
    assert(!frames_.empty());
    if (frameNumber < frames_.size()) {
        return &frames_[frameNumber];
    }
    return 0;
}
//returns the length of this Universe's timeline
size_t Universe::getTimelineLength() const
{
    assert(!frames_.empty());
    return frames_.size();
}
Frame* Universe::getFrame(const FrameID& whichFrame)
{
    assert(getTimelineLength()==whichFrame.universe().timelineLength());
    assert(!initiatorFrame_);
    Frame* parentFrame(0);
    for (std::vector<SubUniverse>::const_iterator it(whichFrame.universe().nestTrain_.begin()), end(whichFrame.universe().nestTrain_.end()); it != end; ++it)
    {
        if (!parentFrame) {
            parentFrame = getArbitraryFrame(it->initiatorFrame_);
        }
        else {
            parentFrame = parentFrame->arbitraryChildFrame((it-1)->pauseInitiatorID_, (it)->initiatorFrame_);
        }
    }
    if (parentFrame) {
        return parentFrame->arbitraryChildFrame(whichFrame.universe().nestTrain_.rbegin()->pauseInitiatorID_, whichFrame.frame());
    }
    else {
        return getArbitraryFrame(whichFrame.frame());
    }
}
Universe::Universe(Frame* initiatorFrame, size_t timelineLength, const PauseInitiatorID& initiatorID) :
initiatorFrame_(0),
frames_()
{
    assert(timelineLength > 0);
    construct(initiatorFrame, timelineLength, initiatorID);
}

void Universe::construct(Frame* initiatorFrame, size_t timelineLength, const PauseInitiatorID& initiatorID)
{
    initiatorFrame_ = initiatorFrame;
    frames_.reserve(timelineLength);
    for (unsigned int i(0); i < timelineLength; ++i)
    {
        frames_.push_back(Frame(i, *this));
    }
    initiatorID_ = &initiatorID;
}
}