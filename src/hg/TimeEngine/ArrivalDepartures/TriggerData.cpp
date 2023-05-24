#include "TriggerData.h"
#include <limits>
#include <cassert>
#include <tuple>
namespace hg {
TriggerData::TriggerData(std::size_t index, mt::std::vector<int> const &value) :
index(index),
value(value)
{
	assert(index != std::numeric_limits<std::size_t>::max()
	    && "the max value is reserved for representing invalid/null indices");
}

std::weak_ordering TriggerData::operator<=>(TriggerData const &o) const
{
	//TriggerData sorted by index only.
	return index <=> o.index;
}
}
