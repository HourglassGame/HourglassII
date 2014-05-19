#include "TriggerData.h"
#include <limits>
#include <cassert>
namespace hg {
TriggerData::TriggerData(std::size_t index, mt::std::vector<int> const &value) :
index(index),
value(value)
{
    assert(index != std::numeric_limits<std::size_t>::max()
            && "the max value is reserved for representing invalid/null indices");
}

bool TriggerData::operator==(TriggerData const &o) const
{
    return std::tie(index, value) == std::tie(o.index, o.value);
}

bool TriggerData::operator<(TriggerData const &o) const
{
    //TriggerData sorted by index only.
    return index < o.index;
}
}
