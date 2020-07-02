#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H

#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"

#include "hg/TimeEngine/ArrivalDepartures/ObjectList.h"
#include "hg/TimeEngine/ArrivalDepartures/ObjectListTypes.h"
#include "hg/mt/std/map"
#include "hg/mt/boost/unordered_map.hpp"


namespace hg {
class DepartureMap final {
	typedef
	mt::boost::unordered_map<
		Frame *,
		mt::std::map<Frame *, ObjectList<Normal>>
	> MapType;
public:
	typedef MapType::value_type value_type;
	typedef MapType::iterator iterator;
	typedef MapType::const_iterator const_iterator;
	//MUST be called with all the times which will be passed to addDeparture before calling addDeparture
	void makeSpaceFor(FrameUpdateSet const &toMakeSpaceFor, unsigned speedOfTimeFilter,
		unsigned futureSpeedOfTimeLimit, unsigned guyFrameNumber, TimeDirection guyDirection);
	void setDeparture(Frame *frame, MapType::mapped_type &&departingObjects);
	iterator begin();
	iterator end();
	unsigned size() const;
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
private:
	MapType map;
};
}

#endif //HG_DEPARTURE_MAP_H
