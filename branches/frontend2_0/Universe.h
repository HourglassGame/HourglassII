#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"

#include <vector>

namespace hg {
class Universe {
public:
    ///creates a top level universe
    Universe(unsigned int timelineLength);
    ///creates a lower level universe
    Universe(FramePtr initiatorFrame, unsigned int timelineLength);
    ///returns initiatorFrame_
    FramePtr getInitiatorFrame() const;
    FramePtr getEntryFrame(TimeDirection direction) const;
    FramePtr getArbitraryFrame(unsigned int frameNumber) const;
    ///returns the length of this Universe's timeline
    unsigned int getTimelineLength() const;
private:
    FramePtr initiatorFrame_;
    ::std::vector<Frame> frames;
};

::std::size_t hash_value(const Universe& toHash);
}
#endif //HG_UNIVERSE_H
