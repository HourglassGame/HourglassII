#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap.h"
#include "ParallelForEach.h"
#include "Frame.h"
#include "FrameID.h"
#include "ConcurrentFrameUpdateSet.h"
#include "Foreach.h"
#include <boost/swap.hpp>
#include <cassert>
#include <algorithm>

namespace hg {
TimelineState::TimelineState(std::size_t timelineLength) :
        universe_(timelineLength)
{
}

void TimelineState::swap(TimelineState& other)
{
    boost::swap(universe_, other.universe_);
    boost::swap(permanentDepartures_, other.permanentDepartures_);
}

struct UpdateDeparturesFromFrame
{
    UpdateDeparturesFromFrame(ConcurrentFrameUpdateSet& framesWithChangedArrivals) :
            framesWithChangedArrivals_(framesWithChangedArrivals)
    {}
    void operator()(DepartureMap::value_type& newDeparture) const {
        framesWithChangedArrivals_.add(newDeparture.first->updateDeparturesFromHere(newDeparture.second));
    }
    ConcurrentFrameUpdateSet& framesWithChangedArrivals_;
};

FrameUpdateSet
TimelineState::updateWithNewDepartures(
	DepartureMap& newDepartures/*, tbb::task_group_context& context*/)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(
    	newDepartures,
    	UpdateDeparturesFromFrame(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}
void TimelineState::addArrivalsFromPermanentDepartureFrame(
		std::map<Frame*, ObjectList<Normal> > const& initialArrivals)
{
    foreach (auto const& arrival, initialArrivals) {
        permanentDepartures_[arrival.first].add(arrival.second);
        permanentDepartures_[arrival.first].sort();
        arrival.first->addArrival(0, &(permanentDepartures_[arrival.first]));
    }
}
Frame* TimelineState::getFrame(FrameID const& whichFrame)
{
    return universe_.getFrame(whichFrame);
}
}//namespace hg
