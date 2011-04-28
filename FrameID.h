#ifndef HG_FRAME_ID_H
#define HG_FRAME_ID_H

#include "TimeDirection.h"
#include "UniverseID.h"

#include <cstddef>
#include <cassert>

namespace hg {
class Frame;
//Class following original intention of FrameID. May be too slow for back-end use,
//Compliments Frame* by not requiring a central authority (ie the base universe) to be used
class FrameID {
public:
    //Creates a nullframe
    FrameID();

    //Creates a FrameID referring to the given time in the given universe
    FrameID(std::size_t time, const UniverseID& universe);

    //Creates a FrameID corresponding to the given Frame*
    explicit FrameID(const Frame* toConvert);
    
    // returns the normal next frame for things moving in direction TimeDirection
    FrameID nextFrame(TimeDirection direction) const;

    // returns the number of universes up that the next frame is
    // (0 for in same universe)
    unsigned int nextFramePauseLevelDifference(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FrameID arbitraryFrameInUniverse(std::size_t frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    FrameID parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater pauseLength
    FrameID arbitraryChildFrame(const PauseInitiatorID& initatorID, std::size_t frameNumber) const;

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    FrameID entryChildFrame(const PauseInitiatorID& initatorID, TimeDirection direction) const;

    bool operator==(const FrameID& other) const;
    bool operator!=(const FrameID& other) const;

    bool operator<(const FrameID& other) const;

    bool isValidFrame() const;

    std::size_t frame() const {
        assert (isValidFrame());
        return frame_;
    }

    const UniverseID& universe() const {
        return universeID_;
    }

private:
    friend std::size_t hash_value(const FrameID& toHash);
    //positiong of frame within universeID_
    std::size_t frame_;
    UniverseID universeID_;
    unsigned int nextFramePauseLevelDifferenceAux(unsigned int depthAccumulator, TimeDirection direction) const;
};
//Returns a std::size_t based on toHash such that two FrameIDs for which operator== returns true give the same std::size_t value;
std::size_t hash_value(const FrameID& toHash);
}//namespace hg
#endif //HG_FRAME_ID_H
