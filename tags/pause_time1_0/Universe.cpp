#include "Universe.h"
#include "FrameID.h"

#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <functional>
#include <cassert>
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
    (void)other;
    //Ok to copy construct as long as there are no frames which know where the universe is
    //and there are no frames which know where the frames in this universe are
    assert(other.frames_.empty());
}
//creates a top level universe
Universe::Universe(std::size_t timelineLength) :
        initiatorFrame_(0),
        frames_(),
        initiatorID_(0)
{
    assert(timelineLength > 0);
    construct(initiatorFrame_, timelineLength, *initiatorID_);
}
//returns initiatorFrame_
Frame const* Universe::getInitiatorFrame() const
{
    assert(!frames_.empty());
    return initiatorFrame_;
}
//returns initiatorFrame_
Frame* Universe::getInitiatorFrame()
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
PauseInitiatorID const& Universe::getInitiatorID() const
{
    assert(initiatorID_);
    return *initiatorID_;
}

//Returns the frame which this Universe is a sub universe of.
//The top level universe is a sub universe of the NullFrame
Frame const* getInitiatorFrame(Universe const& universe)
{
    return universe.getInitiatorFrame();
}

//Returns the frame which this Universe is a sub universe of.
//The top level universe is a sub universe of the NullFrame
Frame* getInitiatorFrame(Universe& universe)
{
    return universe.getInitiatorFrame();
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
//Returns the ID of the initiator of the sub-universe
//If this is the main universe then behaviour is undefined
PauseInitiatorID const& getInitiatorID(Universe const& universe)
{
    return universe.getInitiatorID();
}


Frame* Universe::getFrame(const FrameID& whichFrame)
{
    assert(getTimelineLength() == whichFrame.universe().timelineLength());
    assert(!initiatorFrame_);
    Frame* parentFrame(0);
    for (std::vector<SubUniverse>::const_iterator it(whichFrame.universe().nestTrain_.begin()),
            end(whichFrame.universe().nestTrain_.end()); it != end; ++it)
    {
        if (!parentFrame) {
            parentFrame = hg::getArbitraryFrame(*this, it->initiatorFrame_);
        }
        else {
            parentFrame = 
                hg::getArbitraryFrame(
                    getSubUniverse(
                        parentFrame,
                        (it-1)->pauseInitiatorID_),
                    (it)->initiatorFrame_);
        }
    }
    if (parentFrame) {
        return hg::getArbitraryFrame(
                    getSubUniverse(
                        parentFrame,
                        whichFrame.universe().nestTrain_.rbegin()->pauseInitiatorID_),
                    whichFrame.getFrameNumber());
    }
    else {
        return getArbitraryFrame(whichFrame.getFrameNumber());
    }
}
Universe::Universe(Frame* initiatorFrame, std::size_t timelineLength, PauseInitiatorID const& initiatorID) :
        initiatorFrame_(0),
        frames_()
{
    assert(timelineLength > 0);
    construct(initiatorFrame, timelineLength, initiatorID);
}
namespace {
struct ConstructFrame : std::unary_function<std::size_t, Frame> {
    ConstructFrame(Universe& universe) : universe_(universe) {}
    Frame operator()(std::size_t frameNumber) const {
        return Frame(frameNumber, universe_);
    }
private:
    Universe& universe_;
};
}

void Universe::construct(Frame* initiatorFrame, std::size_t timelineLength, const PauseInitiatorID& initiatorID)
{
    assert(initiatorFrame_ == 0 && "Trying to construct already constructed universe!");
    assert(frames_.empty() && "Trying to construct already constructed universe!");
    assert(initiatorID_ == 0 && "Trying to construct already constructed universe!");
    initiatorFrame_ = initiatorFrame;
    boost::push_back(frames_, boost::irange<std::size_t>(0, timelineLength)
                     | boost::adaptors::transformed(ConstructFrame(*this)));
    initiatorID_ = &initiatorID;
}
}
