#include "DepartureMap.h"
#include "FrameUpdateSet.h"
#include <utility>
namespace hg {
void DepartureMap::makeSpaceFor(FrameUpdateSet const &toMakeSpaceFor, unsigned speedOfTimeFilter,
		unsigned futureSpeedOfTimeLimit, unsigned guyFrameNumber, TimeDirection guyDirection)
{
	size_t spaceRequired = 0;
	for (Frame *frame : toMakeSpaceFor) {
		if (speedOfTimeFilter < getFrameSpeedOfTime(frame) &&
				(futureSpeedOfTimeLimit > speedOfTimeFilter ||
					guyDirection == TimeDirection::INVALID ||
					(guyDirection == TimeDirection::FORWARDS && getFrameNumber(frame) <= guyFrameNumber) ||
					(guyDirection == TimeDirection::REVERSE && getFrameNumber(frame) >= guyFrameNumber))) {
			++spaceRequired;
		}
	}
	map.rehash(spaceRequired);
	//removes the need for locking in addDeparture by making a map with spaces for all the items in toMakeSpaceFor
	for (Frame *frame: toMakeSpaceFor)
	{
		if (speedOfTimeFilter < getFrameSpeedOfTime(frame) &&
				(futureSpeedOfTimeLimit > speedOfTimeFilter ||
					guyDirection == TimeDirection::INVALID ||
					(guyDirection == TimeDirection::FORWARDS && getFrameNumber(frame) <= guyFrameNumber) ||
					(guyDirection == TimeDirection::REVERSE && getFrameNumber(frame) >= guyFrameNumber))) {
			map.insert(value_type(frame, MapType::mapped_type()));
		}
	}
}
void DepartureMap::setDeparture(Frame *frame, MapType::mapped_type &&departingObjects)
{
	map.find(frame)->second = std::move(departingObjects);
}

unsigned DepartureMap::size() const
{
	return static_cast<unsigned>(map.size());
}
DepartureMap::iterator DepartureMap::begin()
{
	return map.begin();
}
DepartureMap::iterator DepartureMap::end()
{
	return map.end();
}
DepartureMap::const_iterator DepartureMap::begin() const
{
	return map.begin();
}
DepartureMap::const_iterator DepartureMap::end() const
{
	return map.end();
}
DepartureMap::const_iterator DepartureMap::cbegin() const
{
	return begin();
}
DepartureMap::const_iterator DepartureMap::cend() const
{
	return end();
}
}
