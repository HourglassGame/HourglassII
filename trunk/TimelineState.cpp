#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap_def.h"
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
    void operator()(DepartureMap<ObjectList<Normal> >::value_type& newDeparture) const
    {
        framesWithChangedArrivals_.add(newDeparture.first->updateDeparturesFromHere(newDeparture.second));
    }
    ConcurrentFrameUpdateSet& framesWithChangedArrivals_;
};

struct UpdateEditDeparturesFromTime
{
    UpdateEditDeparturesFromTime(ConcurrentFrameUpdateSet& framesWithChangedArrivals) :
            framesWithChangedArrivals_(framesWithChangedArrivals)
    {
    }
    void operator()(DepartureMap<ObjectList<Edit> >::value_type& newDeparture) const
    {
        framesWithChangedArrivals_.add(newDeparture.first->updateEditDeparturesFromHere(newDeparture.second));
    }
    ConcurrentFrameUpdateSet& framesWithChangedArrivals_;
};

TimelineState::TimelineState(std::size_t timelineLength) :
        universe_(timelineLength)
{
}

FrameUpdateSet TimelineState::updateWithNewDepartures(DepartureMap<ObjectList<Normal> >& newDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(newDepartures, UpdateDeparturesFromTime(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}

FrameUpdateSet TimelineState::updateWithNewEditDepartures(DepartureMap<ObjectList<Edit> >& newDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(newDepartures, UpdateEditDeparturesFromTime(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}

void TimelineState::addArrivalsFromPermanentDepartureFrame(std::map<Frame*, ObjectList<Normal> >& initialArrivals)
{
    for (std::map<Frame*, ObjectList<Normal> >::const_iterator it(initialArrivals.begin()), end(initialArrivals.end()); it != end; ++it) {
        permanentDepartures_[it->first].add(it->second);
        permanentDepartures_[it->first].sort();
        it->first->addArrival(0, &(permanentDepartures_[it->first]));
    }
}

Frame* TimelineState::getFrame(const FrameID& whichFrame)
{
    return universe_.getFrame(whichFrame);
}
}//namespace hg
