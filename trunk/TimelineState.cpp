#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap.h"
#include "ParallelForEach.h"
#include "Frame.h"
#include "FrameID.h"
#include "ConcurrentFrameUpdateSet.h"

#include <cassert>
#include <algorithm>

namespace hg {
struct UpdateDeparturesFromTime
{
    UpdateDeparturesFromTime(ConcurrentFrameUpdateSet& framesWithChangedArrivals) :
            framesWithChangedArrivals_(framesWithChangedArrivals)
    {
    }
    void operator()(DepartureMap::value_type& newDeparture) const
    {
        framesWithChangedArrivals_.add(newDeparture.first->updateDeparturesFromHere(boost::move(newDeparture.second)));
    }
    ConcurrentFrameUpdateSet& framesWithChangedArrivals_;
};
TimelineState::TimelineState(std::size_t timelineLength) :
        universe_(timelineLength)
{
}
TimelineState::TimelineState(BOOST_RV_REF(TimelineState) other) :
    universe_(boost::move(other.universe_)),
    permanentDepartures_(boost::move(other.permanentDepartures_))
{
}
TimelineState& TimelineState::operator=(BOOST_RV_REF(TimelineState) other)
{
    universe_ = boost::move(other.universe_);
    permanentDepartures_= boost::move(other.permanentDepartures_);
    return *this;
}
FrameUpdateSet TimelineState::updateWithNewDepartures(DepartureMap& newDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(newDepartures, UpdateDeparturesFromTime(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}
void TimelineState::addArrivalsFromPermanentDepartureFrame(boost::container::map<Frame*, ObjectList<Normal> > const& initialArrivals)
{
    for (boost::container::map<Frame*, ObjectList<Normal> >::const_iterator it(initialArrivals.begin()), end(initialArrivals.end()); it != end; ++it) {
        permanentDepartures_[it->first].add(it->second);
        permanentDepartures_[it->first].sort();
        it->first->addArrival(0, &(permanentDepartures_[it->first]));
    }
}
Frame* TimelineState::getFrame(FrameID const& whichFrame)
{
    return universe_.getFrame(whichFrame);
}
}//namespace hg
