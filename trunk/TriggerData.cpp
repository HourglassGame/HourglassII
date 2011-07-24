#include "TriggerData.h"
#include <limits>
#include <cassert>
namespace hg {
TriggerData::TriggerData(std::size_t index, mt::boost::container::vector<int>::type const& value) :
index_(index),
value_(value)
{
    assert(index_ != std::numeric_limits<std::size_t>::max()
            && "the max value is reserved for representing invalid/null indices");
}

bool TriggerData::operator==(const TriggerData& other) const
{
    return (index_ == other.index_) && (value_ == other.value_);
}

bool TriggerData::operator<(const TriggerData& other) const
{
    return index_ < other.index_;
}
}
