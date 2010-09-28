#include "UniverseID.h"
#include "NewFrameID.h"

#include <boost/functional/hash.hpp>

using namespace ::std;
namespace hg {
SubUniverse::SubUniverse(unsigned int initiatorFrame, const PauseInitiatorID& pauseInitiatorID) :
initiatorFrame_(initiatorFrame),
pauseInitiatorID_(pauseInitiatorID)
{
}

bool operator==(const SubUniverse& lhs, const SubUniverse& rhs)
{
    return lhs.initiatorFrame_ == rhs.initiatorFrame_ 
            && lhs.pauseInitiatorID_ == rhs.pauseInitiatorID_;
}

bool operator<(const SubUniverse& lhs, const SubUniverse& rhs)
{
    if (lhs.initiatorFrame_ == rhs.initiatorFrame_) {
        return lhs.pauseInitiatorID_ < rhs.pauseInitiatorID_;
    }
    else {
        return lhs.initiatorFrame_ < rhs.initiatorFrame_;
    }
}

::std::size_t hash_value(const SubUniverse& toHash)
{
    size_t seed(0);
    ::boost::hash_combine(seed, toHash.initiatorFrame_);
    ::boost::hash_combine(seed, toHash.pauseInitiatorID_);
    return seed;
}

UniverseID::UniverseID(unsigned int ntimelineLength, ::std::vector<SubUniverse> nestTrain) :
timelineLength_(ntimelineLength),
nestTrain_(nestTrain)
{
}

NewFrameID UniverseID::parentFrame() const
{
    if (nestTrain_.empty()) {
        return NewFrameID();
    }
    else {
        return NewFrameID
               (
                   nestTrain_.rbegin()->initiatorFrame_,
                   UniverseID
                   (
                       timelineLength_,
                       ::std::vector<SubUniverse>
                       (
                           nestTrain_.begin(),
                           nestTrain_.end() - 1
                       )
                   )
               );
    }
}

unsigned int UniverseID::timelineLength() const
{
    if (nestTrain_.empty()) {
        return timelineLength_;
    }
    else {
        return nestTrain_.rbegin()->pauseInitiatorID_.timelineLength_;
    }
}

bool UniverseID::operator==(const UniverseID& other) const
{
    return timelineLength_ == other.timelineLength_
            && nestTrain_ == other.nestTrain_;
}

bool UniverseID::operator<(const UniverseID& other) const
{
    if (nestTrain_.size() == other.nestTrain_.size()) {
        for (::std::vector<SubUniverse>::const_reverse_iterator it(nestTrain_.rbegin()), end(nestTrain_.rend()), oit(other.nestTrain_.rbegin());
             it != end; ++it, ++oit) {
            if (*it != *oit) {
                return *it < *oit;
            }
        }
        return timelineLength_ < other.timelineLength_;
    }
    else {
        return nestTrain_.size() < other.nestTrain_.size();
    }
}

UniverseID UniverseID::getSubUniverse(const SubUniverse& newestNest) const
{
    vector<SubUniverse> newNestTrain(nestTrain_);
    newNestTrain.push_back(newestNest);
    return UniverseID(timelineLength_, newNestTrain);
}

::std::size_t hash_value(const UniverseID& toHash)
{
    size_t seed(0);
    ::boost::hash_combine(seed, toHash.timelineLength_);
    ::boost::hash_combine(seed, toHash.nestTrain_);
    return seed;
}

PauseInitiatorID UniverseID::initiatorID() const
{
    if (!nestTrain_.empty()) {
        return nestTrain_.rbegin()->pauseInitiatorID_;
    }
    else {
        return PauseInitiatorID(pauseinitiatortype::INVALID,0,0);
    }

}
size_t UniverseID::pauseDepth() const
{
    return nestTrain_.size();
}
}

