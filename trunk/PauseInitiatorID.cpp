#include "PauseInitiatorID.h"
#include <boost/functional/hash.hpp>

namespace hg {
PauseInitiatorID::PauseInitiatorID(pauseinitiatortype::PauseInitiatorType type,
                                   std::size_t ID,
                                   std::size_t timelineLength) :
        type_(type),
        ID_(ID),
        timelineLength_(timelineLength)
{
}

bool PauseInitiatorID::operator==(const PauseInitiatorID& other) const
{
    return type_ == other.type_
           && ID_ == other.ID_
           && timelineLength_ == other.timelineLength_;
}

bool PauseInitiatorID::operator<(const PauseInitiatorID& other) const
{
    if (type_ == other.type_) {
        if (ID_ == other.ID_) {
            return timelineLength_ < other.timelineLength_;
        }
        else {
            return ID_ < other.ID_;
        }
    }
    else {
        return type_ < other.type_;
    }
}

std::size_t hash_value(const PauseInitiatorID& toHash)
{
    std::size_t seed(0);
    boost::hash_combine(seed, toHash.type_);
    boost::hash_combine(seed, toHash.ID_);
    boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
}//namespace hg
