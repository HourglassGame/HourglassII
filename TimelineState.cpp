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
        framesWithChangedArrivals_.add(newDeparture.first->updateDeparturesFromHere(newDeparture.second));
    }
    ConcurrentFrameUpdateSet& framesWithChangedArrivals_;
};

struct UpdateEditDeparturesFromTime
{
    UpdateEditDeparturesFromTime(ConcurrentFrameUpdateSet& framesWithChangedEditArrivals) :
            framesWithChangedEditArrivals_(framesWithChangedEditArrivals)
    {
    }
    void operator()(EditDepartureMap::value_type& newDeparture) const
    {
        framesWithChangedEditArrivals_.add(newDeparture.first->updateEditDeparturesFromHere(newDeparture.second));
    }
    ConcurrentFrameUpdateSet& framesWithChangedEditArrivals_;
};

struct SetNewRawDepartures
{
    SetNewRawDepartures(ConcurrentFrameUpdateSet& framesWithChangedRawDepartures) :
            framesWithChangedRawDepartures_(framesWithChangedRawDepartures)
    {
    }
    void operator()(RawDepartureMap::value_type& newRawDeparture) const
    {
        if (newRawDeparture.first->getRawDepartures() != newRawDeparture.second) {
            FrameUpdateSet frame;
            frame.add(newRawDeparture.first);
            framesWithChangedRawDepartures_.add(frame);
            newRawDeparture.first->setRawDepartures(newRawDeparture.second);
        }
    }
    ConcurrentFrameUpdateSet& framesWithChangedRawDepartures_;
};
FrameUpdateSet TimelineState::setNewRawDepartures(RawDepartureMap& newRawDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedRawDepartures;
    parallel_for_each(newRawDepartures, SetNewRawDepartures(framesWithChangedRawDepartures));
    return framesWithChangedRawDepartures.merge();
}


TimelineState::TimelineState(std::size_t timelineLength) :
        universe_(timelineLength)
{
}

FrameUpdateSet TimelineState::updateWithNewDepartures(DepartureMap& newDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(newDepartures, UpdateDeparturesFromTime(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}

FrameUpdateSet TimelineState::updateWithNewEditDepartures(EditDepartureMap& newDepartures)
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

Frame* TimelineState::getFrame(FrameID const& whichFrame)
{
    return universe_.getFrame(whichFrame);
}
}//namespace hg
