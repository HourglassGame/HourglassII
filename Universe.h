#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"
#include "FrameID_fwd.h"
#include "TimeDirection.h"
#include <vector>

#include "PauseInitiatorID_fwd.h"

namespace hg {
class Universe {
public:
    //This copy constructor only exists to allow universes to be put into standard containers
    //(which require copy constructors). It only works correctly when copy-constructing
    //a default-constructed universe, and is meant to be used for 2-part construction as a workaround
    //until emplace() is more widely supported.
    //Precondition: other is a default constructed universe
    //    (which has not yet been fully constructed using Universe::construct)
    //Postcondition: *this is a newly constructed universe which has also not been fully constructed.
    Universe(const Universe& other);
    //creates a top level universe
    explicit Universe(std::size_t timelineLength);
    
    //Conversion from FrameID to equivalent Frame* within this universe
    //The universe must be a top level universe, and the FrameID must
    //correspond to a frame that could actually be in the universe.
    Frame* getFrame(const FrameID& whichFrame);
private:
    friend class Frame;
    friend class UniverseID;
    
    //private to enforce use of non-member variants
    //(this is not as important for Universe as for Frame*, but anyway)
    //<UniverseT interface>
    //Returns the frame which this Universe is a sub universe of.
    //The top level universe is a sub universe of the NullFrame
    Frame const* getInitiatorFrame() const;
    Frame* getInitiatorFrame();
    //Returns the first frame in the universe for objects travelling
    //in TimeDirection direction.
    Frame* getEntryFrame(TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe, 
    //or the NullFrame if no such frame exists
    Frame* getArbitraryFrame(std::size_t frameNumber);
    //returns the length of this Universe's timeline
    std::size_t getTimelineLength() const;
    //Returns the ID of the initiator of the sub-universe
    //If this is the main universe then behaviour is undefined
    PauseInitiatorID const& getInitiatorID() const;
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
    //Returns the ID of the initiator of the sub-universe
    //If this is the main universe then behaviour is undefined
    const PauseInitiatorID& getInitiatorID(Universe const& universe);
    
    friend Frame* getInitiatorFrame(Universe& universe);
    friend Frame const* getInitiatorFrame(Universe const& universe);
    friend Frame* getEntryFrame(Universe& universe, TimeDirection direction);
    friend Frame* getArbitraryFrame(Universe& universe, std::size_t frameNumber);
    friend std::size_t getTimelineLength(Universe const& universe);
    friend const PauseInitiatorID& getInitiatorID(Universe const& universe);
    
    //Creates a lower level universe.
    //This is never used in the current code, but kept because it is logically part of the interface,
    //and will be needed once emplace becomes more widely supported and we change away from
    //the 2-part construction workaround.
    Universe(Frame* initiatorFrame, std::size_t timelineLength, const PauseInitiatorID& initiatorID);

    //The default constructor creates an Universe which can be copied around, but which is not
    //useable for anything. construct(...) must be called on a default constructed universe
    //to complete its construction.
    //This "2-part-construction" is meant to allow universes to be constructed in-place within
    //standard containers, as a workaround until emplace() is more widely supported.

    //Specifically - sub-universes exist in a map (key: initiatorID, value: universe) within their parent Frames;
    //Adding elements to a map requires copy construction, but a constructed universe cannot be copied
    //(as this would invalidate every Frame* to every Frame in the Universe)
    //(it would also invalidate the universe_ member of all the Frames the Universe that was copied)
    //The only time that a universe can safely be copied is when it contains no frames (that is, it is
    //in a formative, pre-constructed state). This is exactly what this default constructor creates.
    //Once a universe is in the map it is never again moved, and so it can be fully constructed.
    //This is what construct does.
    Universe();
    //Precondition: Universe was default constructed or copy constructed
    //(from a default constructed Universe) and has had no member functions called
    //since construction.
    void construct(Frame* initiatorFrame, std::size_t timelineLength, const PauseInitiatorID& initiatorID);

    //Intentionally undefined
    Universe& operator=(const Universe&);

    //This is a reference to allow rebinding when performing 2-part construction.
    Frame* initiatorFrame_;
    std::vector<Frame> frames_;
    const PauseInitiatorID* initiatorID_;
};
}
#endif //HG_UNIVERSE_H
