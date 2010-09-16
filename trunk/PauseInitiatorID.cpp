#include "PauseInitiatorID.h"
using namespace ::hg;
bool ::hg::operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs)
{
    return lhs.type_ == rhs.type_ 
        && lhs.ID_ == rhs.ID_
        && lhs.timelineLength_ == rhs.timelineLength_;
}

bool ::hg::operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs)
{
    if (lhs.type_ == rhs.type_) {
        if (lhs.ID_ == rhs.ID_) {
            return lhs.timelineLength_ < rhs.timelineLength_;
        }
        else {
            lhs.ID_ < rhs.ID_;
        }
    }
    else {
        lhs.type_ < rhs.type_;
    }
}

::std::size_t hg::hash_value(const PauseInitiatorID& toHash)
{
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.type_);
    ::boost::hash_combine(seed, toHash.ID_);
    ::boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
