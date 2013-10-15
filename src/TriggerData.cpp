#include "TriggerData.h"
#include <limits>
#include <cassert>
namespace hg {
TriggerData::TriggerData(std::size_t index, mt::std::vector<int>::type const &value) :
index_(index),
value_(value)
{
    assert(index_ != std::numeric_limits<std::size_t>::max()
            && "the max value is reserved for representing invalid/null indices");
}

bool TriggerData::operator==(const TriggerData &o) const
{
    return (index_ == o.index_) && (value_ == o.value_);
}

bool TriggerData::operator<(const TriggerData &o) const
{
    return index_ < o.index_;
}
}
