#ifndef HG_UNIVERSE_ID_H
#define HG_UNIVERSE_ID_H

#include "PauseInitiatorID.h"

#include <vector>
namespace hg {
class NewFrameID;
//Identifies the position of a SubUniverse, 
//this includes the frame in which is exists, as well as the ID of the initiator of the universe 
//(which includes the timelineLength of the subuniverse)
struct SubUniverse {
    SubUniverse(unsigned int initiatorFrame, const PauseInitiatorID& pauseInitiatorID);
    unsigned int initiatorFrame_;
    PauseInitiatorID pauseInitiatorID_;
    private:
    
    SubUniverse(){}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & initiatorFrame_;
        ar & pauseInitiatorID_;
    }
};
bool operator==(const SubUniverse& lhs, const SubUniverse& rhs);
inline bool operator!=(const SubUniverse& lhs, const SubUniverse& rhs)
{
    return !(lhs == rhs);
}
    
bool operator<(const SubUniverse& lhs, const SubUniverse& rhs);
    
::std::size_t hash_value(const SubUniverse& toHash);
    
//Uniquely identifies a particular universe.
//for top-level universe nestTrain_.empty() == true
//timelineLength always refers to the timelinelength of the top-level universe,
struct UniverseID {
    //lower level universeID
    UniverseID(unsigned int timelineLength, ::std::vector<SubUniverse> nestTrain = ::std::vector<SubUniverse>());
    
    PauseInitiatorID initiatorID() const;
    
    ::std::size_t pauseDepth() const;
    
    //returns the frame in which the lowest level of this universe exists. NullFrame if this is a top-level universe.
    NewFrameID parentFrame() const;
    //returns the length of the lowest level of this universe
    unsigned int timelineLength() const;
    
    UniverseID getSubUniverse(const SubUniverse& newestNest) const;
    
    bool operator==(const UniverseID& other) const;
    bool operator<(const UniverseID& other) const;
private:
    friend ::std::size_t hash_value(const UniverseID& toHash);
    unsigned int timelineLength_;
    //nestTrain_[0] is the least nested SubUniverse, and nestTrain_[nestTrain_.size()-1] is the most nested (bottom level)
    ::std::vector<SubUniverse> nestTrain_;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & timelineLength_;
        ar & nestTrain_;
    }
};
::std::size_t hash_value(const UniverseID& toHash);
}
#endif //HG_UNIVERSE_ID_H
