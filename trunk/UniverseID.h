#ifndef HG_UNIVERSE_ID_H
#define HG_UNIVERSE_ID_H

#include "PauseInitiatorID.h"
#include <boost/range.hpp>
#include <vector>

#include "FrameID_fwd.h"
#include "Universe_fwd.h"

namespace hg {
//Identifies the position of a SubUniverse,
//this includes the frame in which is exists, as well as the ID of the initiator of the universe
//(which includes the timelineLength of the subuniverse)
struct SubUniverse {
    SubUniverse(std::size_t initiatorFrame, const PauseInitiatorID& pauseInitiatorID);
    std::size_t initiatorFrame_;
    PauseInitiatorID pauseInitiatorID_;
private:
    SubUniverse():
            initiatorFrame_(0),
            pauseInitiatorID_()
    {}
};
bool operator==(const SubUniverse& lhs, const SubUniverse& rhs);
inline bool operator!=(const SubUniverse& lhs, const SubUniverse& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const SubUniverse& lhs, const SubUniverse& rhs);

std::size_t hash_value(const SubUniverse& toHash);

//Uniquely identifies a particular universe.
//for top-level universe nestTrain_.empty() == true
//timelineLength always refers to the timelinelength of the top-level universe,
struct UniverseID {
    //top level universeID
    //timelineLength is always length of top-level universe
    explicit UniverseID(std::size_t timelineLength);

    //lower level universeID
    //timelineLength is always length of top-level universe, nestTrain gives lengths of lower level universes.
    template<typename SinglePassSubUniverseRange>
    UniverseID(std::size_t timelineLength, const SinglePassSubUniverseRange& nestTrain) :
            timelineLength_(timelineLength),
            nestTrain_(boost::begin(nestTrain), boost::end(nestTrain))
    {
    }

    //Construct a UniverseID corresponding to toConvert
    //This conversion loses information, as UniverseIDs do not contain frames, but Universes do.
    explicit UniverseID(const Universe& toConvert);
    PauseInitiatorID const& getInitiatorID() const;

    std::size_t pauseDepth() const;

    //returns the frame in which the lowest level of this universe exists. NullFrame if this is a top-level universe.
    FrameID parentFrame() const;
    //returns the length of the lowest level of this universe
    std::size_t timelineLength() const;

    UniverseID getSubUniverse(const SubUniverse& newestNest) const;

    bool operator==(const UniverseID& other) const;
    bool operator<(const UniverseID& other) const;
private:
    friend class Universe;
    friend class FrameID;
    friend std::size_t hash_value(const UniverseID& toHash);
    //timelineLength_ -- length of main universe (ie top-level universe).
    std::size_t timelineLength_;
    //nestTrain_[0] is the least nested SubUniverse, and nestTrain_[nestTrain_.size()-1] is the most nested (bottom level)
    std::vector<SubUniverse> nestTrain_;
};
}
#endif //HG_UNIVERSE_ID_H
