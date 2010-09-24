#include "PauseInitiatorID.h"
#include <boost/functional/hash.hpp>

namespace hg {
bool operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs)
{
    return lhs.type_ == rhs.type_
        && lhs.ID_ == rhs.ID_
        && lhs.timelineLength_ == rhs.timelineLength_;
}

bool operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs)
{
    if (lhs.type_ == rhs.type_) {
        if (lhs.ID_ == rhs.ID_) {
            return lhs.timelineLength_ < rhs.timelineLength_;
        }
        else {
            return lhs.ID_ < rhs.ID_;
        }
    }
    else {
        return lhs.type_ < rhs.type_;
    }
}

::std::size_t hash_value(const PauseInitiatorID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.type_);
    ::boost::hash_combine(seed, toHash.ID_);
    ::boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
}//namespace hg
