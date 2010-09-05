#include "TimelineState.h"

#include "FrameUpdateSet.h"
#include "DepartureMap.h"

#include <cassert>
#include <algorithm>
#include <iostream>

using namespace ::std;
using namespace ::hg;

typedef TimeObjectListList::ListType::const_iterator Iterator;

TimelineState::TimelineState(unsigned int timeLength) :
permanentDepartureIndex(timeLength),
arrivals(timeLength),
departures(timeLength)
{
}

ObjectList TimelineState::Frame::getPrePhysics() const
{
    return this_.getPrePhysics(time_);
}
FrameID TimelineState::Frame::getTime() const
{
    return time_;
}

TimelineState::Frame::Frame(const TimelineState& mapPtr, FrameID time) :
time_(time),
this_(mapPtr)
{
}

FrameUpdateSet TimelineState::updateWithNewDepartures(const DepartureMap& newDepartures)
{
    FrameUpdateSet newWaveFrames;
    for(DepartureMap::const_iterator
            it(newDepartures.begin()), end(newDepartures.end());
            it != end;
            ++it)
    {
        newWaveFrames.add(updateDeparturesFromTime(it->first, it->second));
    }
    return newWaveFrames;
}

TimelineState::Frame TimelineState::getFrame(FrameID whichFrame) const
{
    return Frame(*this, whichFrame);
}

void TimelineState::setArrivalsFromPermanentDepartureFrame(TimeObjectListList& initialArrivals)
{
    for (TimeObjectListList::const_iterator it(initialArrivals.begin()), end(initialArrivals.end()); it != end; ++it) {
        arrivals.at(it->first).insertObjectList(permanentDepartureIndex, it->second);
    }
}

//returns which frames are changed
FrameUpdateSet TimelineState::updateDeparturesFromTime(const FrameID time, const TimeObjectListList& newDeparture)
{
    FrameUpdateSet changedTimes;

    Iterator ni(newDeparture.begin());
    const Iterator nend(newDeparture.end());
    Iterator oi(departures.at(time).begin());
    const Iterator oend(departures.at(time).end());

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if ((*ni).first < (*oi).first) {
                    arrivals.at((*ni).first).insertObjectList(time, (*ni).second);
                    changedTimes.addFrame((*ni).first);
                    ++ni;
                }
                else if ((*ni).first == (*oi).first) {
                    if ((*ni).second != (*oi).second) {
                        arrivals.at((*ni).first).setObjectList(time, (*ni).second);
                        changedTimes.addFrame((*ni).first);
                    }
                    ++ni;
                    break;
                }
                else {
                    arrivals.at((*oi).first).clearTime(time);
                    changedTimes.addFrame((*oi).first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    arrivals.at((*oi).first).clearTime(time);
                    changedTimes.addFrame((*oi).first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        arrivals.at((*ni).first).insertObjectList(time, (*ni).second);
        changedTimes.addFrame((*ni).first);
        ++ni;
    }
end:
    departures.at(time) = newDeparture;
    return changedTimes;
}

ObjectList TimelineState::getPostPhysics(FrameID time) const
{
    return departures.at(time).getFlattenedVersion();
}

ObjectList TimelineState::getPrePhysics(unsigned int time) const
{
    return arrivals.at(time).getFlattenedVersion();
}
