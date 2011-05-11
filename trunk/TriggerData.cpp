#include "TriggerData.h"
#include <limits>
#include <cassert>
namespace hg {
TriggerData::TriggerData(std::size_t index, int value) :
        index_(index),
        value_(value)
{
    assert(index_ != std::numeric_limits<std::size_t>::max()
           && "the max value is reserved for representing invalid/null indices");
}

TriggerData::TriggerData(const TriggerData& other) :
		index_(other.index_),
		value_(other.value_)
{
}

TriggerData& TriggerData::operator=(const TriggerData& other)
{
    index_ = other.index_;
    value_ = other.value_;
    return *this;
}

bool TriggerData::operator!=(const TriggerData& other) const
{
    return !(*this==other);
}

bool TriggerData::operator==(const TriggerData& other) const
{
    return (index_ == other.index_)
           && (value_ == other.value_);
}

bool TriggerData::operator<(const TriggerData& other) const
{
    return index_ < other.index_;
}
}
