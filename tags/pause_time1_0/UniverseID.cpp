#include "UniverseID.h"
#include "FrameID.h"
#include "Frame.h"

#include <boost/functional/hash.hpp>
#include <boost/range/algorithm/reverse.hpp>

#include "Universe.h"
namespace hg {
SubUniverse::SubUniverse(std::size_t initiatorFrame, PauseInitiatorID const& pauseInitiatorID) :
        initiatorFrame_(initiatorFrame),
        pauseInitiatorID_(pauseInitiatorID)
{
}

bool SubUniverse::operator==(SubUniverse const& other) const
{
    return initiatorFrame_ == other.initiatorFrame_
           && pauseInitiatorID_ == other.pauseInitiatorID_;
}

bool SubUniverse::operator<(const SubUniverse& other) const
{
    if (initiatorFrame_ == other.initiatorFrame_) {
        return pauseInitiatorID_ < other.pauseInitiatorID_;
    }
    else {
        return initiatorFrame_ < other.initiatorFrame_;
    }
}

std::size_t hash_value(const SubUniverse& toHash)
{
    std::size_t seed(0);
    boost::hash_combine(seed, toHash.initiatorFrame_);
    boost::hash_combine(seed, toHash.pauseInitiatorID_);
    return seed;
}

UniverseID::UniverseID(std::size_t timelineLength) :
        timelineLength_(timelineLength),
        nestTrain_()
{
}

UniverseID::UniverseID(const Universe& toConvert) :
        timelineLength_(),
        nestTrain_()
{
    const Universe* universe(&toConvert);
    for (; universe->initiatorFrame_; universe = &(universe->initiatorFrame_->universe_))
    {
        nestTrain_.push_back(SubUniverse(universe->initiatorFrame_->frameNumber_, *(universe->initiatorID_)));
    }
    boost::reverse(nestTrain_);
    timelineLength_ = universe->frames_.size();
}

FrameID UniverseID::parentFrame() const
{
    if (nestTrain_.empty()) {
        return FrameID();
    }
    else {
        return FrameID
               (
                   nestTrain_.rbegin()->initiatorFrame_,
                   UniverseID
                   (
                       timelineLength_,
                       boost::make_iterator_range(nestTrain_, 0, -1)
                   )
               );
    }
}

std::size_t UniverseID::timelineLength() const
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
        for (std::vector<SubUniverse>::const_reverse_iterator 
            it(nestTrain_.rbegin()),
            end(nestTrain_.rend()),
            oit(other.nestTrain_.rbegin());
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
    UniverseID retv(*this);
    retv.nestTrain_.push_back(newestNest);
    return retv;
}

std::size_t hash_value(const UniverseID& toHash)
{
    std::size_t seed(0);
    boost::hash_combine(seed, toHash.timelineLength_);
    boost::hash_combine(seed, toHash.nestTrain_);
    return seed;
}

const PauseInitiatorID& UniverseID::getInitiatorID() const
{
    assert(!nestTrain_.empty());
    return nestTrain_.rbegin()->pauseInitiatorID_;
}
std::size_t UniverseID::pauseDepth() const
{
    return nestTrain_.size();
}
}

