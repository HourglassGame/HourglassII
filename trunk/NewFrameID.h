#ifndef HG_NEW_FRAME_ID_H
#define HG_NEW_FRAME_ID_H

#include "TimeDirection.h"
#include "UniverseID.h"
#include <cstring>

namespace hg {
//Class following original intention of FrameID. May be too slow for back-end use,
//but I think that it is the best place to start because it is much less disruptive to the rest of the engine
//It is 100% compatible with the FramePtr system from the point of view of physics.
//It may be that both system can co-exist.
class NewFrameID {
public:
    //Creates a nullframe
    NewFrameID();

    //Creates a FrameID referring to the given time in the given universe
    NewFrameID(unsigned int time, const UniverseID& universe);


    // returns the normal next frame for things moving in direction TimeDirection
    NewFrameID nextFrame(TimeDirection direction) const;

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    bool nextFrameInUniverse(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    NewFrameID arbitraryFrameInUniverse(unsigned int frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    NewFrameID parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater pauseLength
    NewFrameID arbitraryChildFrame(const PauseInitiatorID& initatorID, unsigned int frameNumber) const;

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    NewFrameID entryChildFrame(const PauseInitiatorID& initatorID, unsigned int pauseLength, TimeDirection direction) const;

    bool operator==(const NewFrameID& other) const;

    bool operator<(const NewFrameID& other) const;

    bool isValidFrame() const;

    unsigned int frame() const {
        assert (isValidFrame());
        return frame_;
    }

private:
    friend ::std::size_t hash_value(const NewFrameID& toHash);
    unsigned int frame_;
    UniverseID universe_;
};
//Returns a size_t based on toHash such that two NewFrameIDs for which operator== returns true give the same size_t value;
::std::size_t hash_value(const NewFrameID& toHash);
}//namespace hg
#endif //HG_NEW_FRAME_ID_H
