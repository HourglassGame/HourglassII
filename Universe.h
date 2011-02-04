#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"

#include <vector>
#include <cassert>

namespace hg {
class Frame;
class FrameID;
class PauseInitiatorID;
class Universe {
public:
    Universe(const Universe& other);
    //creates a top level universe
    explicit Universe(size_t timelineLength);
    //returns initiatorFrame_
    Frame* getInitiatorFrame() const;
    Frame* getEntryFrame(TimeDirection direction);
    Frame* getArbitraryFrame(size_t frameNumber);
    //returns the length of this Universe's timeline
    size_t getTimelineLength() const;
    Frame* getFrame(const FrameID& whichFrame);
    const PauseInitiatorID& getInitiatorID()
    {
        assert(initiatorID_);
        return *initiatorID_;
    }
private:
    friend class Frame;
    //creates a lower level universe
    Universe(Frame* initiatorFrame, size_t timelineLength, const PauseInitiatorID& initiatorID);
    
    //workaround until I have a compiler which supports emplace
    //First build the universe and copy it in to place.
    Universe();
    Universe& operator=(const Universe&);
    //then call construct.
    void construct(Frame* initiatorFrame, size_t timelineLength, const PauseInitiatorID& initiatorID);
    Frame* initiatorFrame_;
    ::std::vector<Frame> frames_;
    const PauseInitiatorID* initiatorID_;
    
};
}
#endif //HG_UNIVERSE_H
