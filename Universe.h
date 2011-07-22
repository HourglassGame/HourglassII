#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"
#include "FrameID_fwd.h"
#include "TimeDirection.h"
#include <boost/move/move.hpp>
#include <boost/container/vector.hpp>
#include <vector>

namespace hg {
class Universe {
public:
    //creates a top level universe
    explicit Universe(std::size_t timelineLength);
    
    Universe(BOOST_RV_REF(Universe) other);
    Universe& operator=(BOOST_RV_REF(Universe) other);
    
    //Conversion from FrameID to equivalent Frame* within this universe
    //whichFrame must correspond to a frame that could actually 
    //be in the universe.
    Frame* getFrame(const FrameID& whichFrame);
private:
    friend class Frame;
    friend class UniverseID;
    void fixFramesUniverses();
    //private to enforce use of non-member variants
    //(this is not as important for Universe as for Frame*, but anyway)
    //<UniverseT interface>
    //Returns the first frame in the universe for objects travelling
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
    //Returns the first frame in the universe for objects travelling
    //in TimeDirection direction.
    Frame* getEntryFrame(Universe& universe, TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe, 
    //or the NullFrame if no such frame exists
    Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
    //returns the length of this Universe's timeline
    std::size_t getTimelineLength(Universe const& universe);
    
    friend Frame* getEntryFrame(Universe& universe, TimeDirection direction);
    friend Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
    friend std::size_t getTimelineLength(Universe const& universe);

    BOOST_MOVABLE_BUT_NOT_COPYABLE(Universe)
    boost::container::vector<Frame> frames_;
};
}
#endif //HG_UNIVERSE_H
