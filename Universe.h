#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"
#include "FrameID_fwd.h"
#include "TimeDirection.h"
#include <vector>
#include "Universe_fwd.h"
namespace hg {
class Universe {
public:
    //creates a top level universe
    explicit Universe(std::size_t timelineLength);
    
    Universe(Universe&& other);
    Universe& operator=(Universe&& other);
    void swap(Universe& other);
    
    //Conversion from FrameID to equivalent Frame* within this universe
    //whichFrame must correspond to a frame that could actually 
    //be in the universe.
    Frame* getFrame(const FrameID& whichFrame);
    
private:
    friend class Frame;
    friend class UniverseID;
    void fixFramesUniverses();
    
        //<UniverseT interface>
    //Returns the first frame in the universe for objects traveling
    //in TimeDirection direction.
    Frame* getEntryFrame(TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe, 
    //or the NullFrame if no such frame exists
    Frame* getArbitraryFrame(std::size_t frameNumber);
    //returns the length of this Universe's timeline
    std::size_t getTimelineLength() const;
    //</UniverseT interface>
    //Returns the frame which this Universe is a sub universe of.
    //The top level universe is a sub universe of the NullFrame
    Frame* getInitiatorFrame(Universe const& universe);
    //Returns the first frame in the universe for objects traveling
    //in TimeDirection direction.
    Frame* getEntryFrame(Universe& universe, TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe, 
    //or the NullFrame if no such frame exists
    Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
    
    friend Frame* getEntryFrame(Universe& universe, TimeDirection direction);
    friend Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
    friend std::size_t getTimelineLength(Universe const& universe);

    std::vector<Frame> frames_;
    
    Universe(Universe const& other) = delete;
    Universe& operator=(Universe const& other) = delete;
};
Frame* getEntryFrame(Universe& universe, TimeDirection direction);
Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
std::size_t getTimelineLength(Universe const& universe);
inline void swap(Universe& l, Universe& r) { l.swap(r); }
}
#endif //HG_UNIVERSE_H
