#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "PauseInitiatorID.h"
#include <boost/unordered_map.hpp>
namespace hg {
class Frame;
class FramePtr;
class Universe;
//Wrapper around Frame* to give extra safety (assert ptr before dereferencing,
//    also allows shared syntax with NewFrameID for ease of changing).
class FramePtr {
public:
    //NullFrame construction
    FramePtr();
    //assignment
    FramePtr& operator=(const FramePtr& other);
    //copy-construction
    FramePtr(const FramePtr& other);
    // returns the normal next frame for things moving in direction TimeDirection
    FramePtr nextFrame(TimeDirection direction) const;

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    bool nextFrameInUniverse(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FramePtr arbitraryFrameInUniverse(unsigned int frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    FramePtr parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater pauseLength
    FramePtr arbitraryChildFrame(const PauseInitiatorID& initatorID, unsigned int frameNumber) const;

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    FramePtr entryChildFrame(const PauseInitiatorID& initatorID, TimeDirection direction) const;

    bool operator==(const FramePtr& other);

    bool operator<(const FramePtr& other);
private:
    friend ::std::size_t hash_value(const FramePtr& toHash);
    friend class Universe;

    //constructs a FramePtr for the frame at time in the universe given by Universe
    FramePtr(unsigned int time, const Universe& universe);

    Frame* framePtr_;
};
//Only one frame per frame. Referenced by frame pointers and contained in universes.
//Tim's idea for FrameID
//A system like this could also put the arrivals and departures in Frames
//and so avoid the arrival-departure-map system altogether
class Frame {
    unsigned int frameNumber_;
    Universe* universe_;
    //Here UniverseID has:
    //InitiatorID
    //Universe has:
    //a container of Frames - the Frames in that universe
    ::boost::unordered_map<PauseInitiatorID, Universe> subUniverses_;
};
}
#endif //HG_FRAME_H
