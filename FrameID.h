#ifndef HG_FRAME_ID_H
#define HG_FRAME_ID_H

#include "TimeDirection.h"
#include "UniverseID.h"

#include <boost/serialization/nvp.hpp>

#include <cstring>
#include <cassert>

namespace hg {
//Class following original intention of FrameID. May be too slow for back-end use,
//but I think that it is the best place to start because it is much less disruptive to the rest of the engine
//It is 100% compatible with the FramePtr system from the point of view of physics.
//It may be that both system can co-exist.
class FrameID {
public:
    //Creates a nullframe
    FrameID();

    //Creates a FrameID referring to the given time in the given universe
    FrameID(unsigned int time, const UniverseID& universe);


    // returns the normal next frame for things moving in direction TimeDirection
    FrameID nextFrame(TimeDirection direction) const;

    // returns the number of universes up that the next frame is
    // (0 for 
    unsigned int nextFramePauseLevelDifference(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FrameID arbitraryFrameInUniverse(unsigned int frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    FrameID parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater pauseLength
    FrameID arbitraryChildFrame(const PauseInitiatorID& initatorID, unsigned int frameNumber) const;

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    FrameID entryChildFrame(const PauseInitiatorID& initatorID, TimeDirection direction) const;

    bool operator==(const FrameID& other) const;
    bool operator!=(const FrameID& other) const;

    bool operator<(const FrameID& other) const;

    bool isValidFrame() const;

    unsigned int frame() const {
        assert (isValidFrame());
        return frame_;
    }

    const UniverseID& universe() const {
        return universe_;
    }

private:
    friend ::std::size_t hash_value(const FrameID& toHash);
    unsigned int frame_;
    UniverseID universe_;
    unsigned int nextFramePauseLevelDifferenceAux(unsigned int depthAccumulator, TimeDirection direction) const;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(frame_);
        ar & BOOST_SERIALIZATION_NVP(universe_);
    }
};
//Returns a size_t based on toHash such that two FrameIDs for which operator== returns true give the same size_t value;
::std::size_t hash_value(const FrameID& toHash);
}//namespace hg
#endif //HG_FRAME_ID_H
