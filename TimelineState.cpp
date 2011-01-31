#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap.h"
#include "ParallelForEach.h"
#include "Frame.h"
#include "FrameID.h"
#include "ConcurrentFrameUpdateSet.h"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace ::std;
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

TimelineState::TimelineState(unsigned int timelineLength) :
universe_(timelineLength)
{
}

FrameUpdateSet TimelineState::updateWithNewDepartures(DepartureMap& newDepartures)
{
    ConcurrentFrameUpdateSet framesWithChangedArrivals;
    parallel_for_each(newDepartures, UpdateDeparturesFromTime(framesWithChangedArrivals));
    return framesWithChangedArrivals.merge();
}
void TimelineState::addArrivalsFromPermanentDepartureFrame(std::map<Frame*, ObjectList>& initialArrivals)
{
    for (std::map<Frame*, ObjectList>::const_iterator it(initialArrivals.begin()), end(initialArrivals.end()); it != end; ++it) {
        permanentDepartures_[it->first].add(it->second);
        permanentDepartures_[it->first].sort();
        it->first->addArrival(0, &(permanentDepartures_[it->first]));
    }
}

Frame* TimelineState::getFrame(const FrameID& whichFrame)
{
    assert(universe_.getTimelineLength()==whichFrame.universe().timelineLength_);
    Frame* parentFrame(0);
    for (std::vector<SubUniverse>::const_iterator it(whichFrame.universe().nestTrain_.begin()), end(whichFrame.universe().nestTrain_.end()); it != end; ++it)
    {
        if (!parentFrame) {
            parentFrame = universe_.getArbitraryFrame(it->initiatorFrame_);
        }
        else {
            parentFrame = parentFrame->arbitraryChildFrame((it-1)->pauseInitiatorID_, (it)->initiatorFrame_);
        }
    }
    if (parentFrame) {
        return parentFrame->arbitraryChildFrame(whichFrame.universe().nestTrain_.rbegin()->pauseInitiatorID_, whichFrame.frame());
    }
    else {
        return universe_.getArbitraryFrame(whichFrame.frame());
    }
}
}//namespace hg
