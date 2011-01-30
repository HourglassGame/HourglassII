#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"

#include <vector>

namespace hg {
class Universe {
public:
    Universe(const Universe& other);
    //creates a top level universe
    explicit Universe(unsigned int timelineLength);
    //returns initiatorFrame_
    Frame* getInitiatorFrame() const;
    Frame* getEntryFrame(TimeDirection direction);
    Frame* getArbitraryFrame(unsigned int frameNumber);
    //returns the length of this Universe's timeline
    unsigned int getTimelineLength() const;
private:
    friend class Frame;
    //creates a lower level universe
    Universe(Frame* initiatorFrame, unsigned int timelineLength);
    
    //workaround until I have a compiler which supports emplace
    //First build the universe and copy it in to place.
    Universe();
    Universe& operator=(const Universe&);
    //then call construct.
    void construct(Frame* initiatorFrame, unsigned int timelineLength);
    Frame* initiatorFrame_;
    ::std::vector<Frame> frames_;
};
}
#endif //HG_UNIVERSE_H
