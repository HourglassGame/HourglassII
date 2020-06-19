#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap.h"
#include "hg/Util/ParallelForEach.h"
#include "Frame.h"
#include "FrameID.h"
#include <boost/swap.hpp>
#include <cassert>
#include <algorithm>
#include <iostream>

namespace hg {
boost::container::map<Frame *, ObjectList<Normal>>
TimelineState::fixPermanentDepartures(
		FramePointerUpdater const &framePointerUpdater,
		boost::container::map<Frame *, ObjectList<Normal>> const &oldPermanentDepartures)
{
	boost::container::map<Frame *, ObjectList<Normal>> newPermanentDepartures;
	for (auto const &departurePair: oldPermanentDepartures) {
		Frame *newFrame = framePointerUpdater.updateFrame(departurePair.first);
		newPermanentDepartures[newFrame] = departurePair.second;
		newFrame->setPermanentArrival(&newPermanentDepartures[newFrame]);
	}
	return newPermanentDepartures;
}

TimelineState::TimelineState(std::size_t timelineLength, unsigned defaultSpeedOfTime) :
		universe_(static_cast<int>(timelineLength), defaultSpeedOfTime)
{
}

void TimelineState::swap(TimelineState &o) noexcept
{
	boost::swap(universe_, o.universe_);
	boost::swap(permanentDepartures_, o.permanentDepartures_);
}

FrameUpdateSet
TimelineState::updateWithNewDepartures(
	DepartureMap &newDepartures,
	ConcurrentFrameUpdateSet &framesWithChangedArrivals)
{
	parallel_for_each(
		newDepartures,
		[&](DepartureMap::value_type &newDeparture)
		{
		//std::cerr << "newDeparture.first: " << getFrameNumber(newDeparture.first) << "\n";
			framesWithChangedArrivals.add(
				newDeparture.first->updateDeparturesFromHere(std::move(newDeparture.second)));
		});
	return framesWithChangedArrivals.merge();
}
void TimelineState::addArrivalsFromPermanentDepartureFrame(
		std::map<Frame *, ObjectList<Normal>> const &initialArrivals)
{
	for (auto const &arrival: initialArrivals) {
		permanentDepartures_[arrival.first].add(arrival.second);
		permanentDepartures_[arrival.first].sort();
		arrival.first->setPermanentArrival(&permanentDepartures_[arrival.first]);
	}
}

}//namespace hg
