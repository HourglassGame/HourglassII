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
//permanentDepartureIndex(timeLength),
arrivals(),
departures()
{
}

ObjectList TimelineState::Frame::getPrePhysics() const
{
    return this_.getPrePhysics(time_);
}
NewFrameID TimelineState::Frame::getTime() const
{
    return time_;
}

TimelineState::Frame::Frame(const TimelineState& mapPtr, NewFrameID time) :
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

TimelineState::Frame TimelineState::getFrame(NewFrameID whichFrame) const
{
    return Frame(*this, whichFrame);
}

void TimelineState::addArrivalsFromPermanentDepartureFrame(const TimeObjectListList& initialArrivals)
{
    for (TimeObjectListList::const_iterator it(initialArrivals.begin()), end(initialArrivals.end()); it != end; ++it) {
        arrivals[it->first].addObjectList(NewFrameID(), it->second);
    }
}

//returns which frames are changed
FrameUpdateSet TimelineState::updateDeparturesFromTime(const NewFrameID time, const TimeObjectListList& newDeparture)
{
    FrameUpdateSet changedTimes;

    Iterator ni(newDeparture.begin());
    const Iterator nend(newDeparture.end());
    Iterator oi(departures[time].begin());
    const Iterator oend(departures[time].end());

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
                        arrivals[(*ni).first].setObjectList(time, (*ni).second);
                        changedTimes.addFrame((*ni).first);
                    }
                    ++ni;
                    break;
                }
                else {
                    arrivals[(*oi).first].clearTime(time);
                    changedTimes.addFrame((*oi).first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    arrivals[(*oi).first].clearTime(time);
                    changedTimes.addFrame((*oi).first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        arrivals[(*ni).first].insertObjectList(time, (*ni).second);
        changedTimes.addFrame((*ni).first);
        ++ni;
    }
end:
    departures[time] = newDeparture;
    return changedTimes;
}

ObjectList TimelineState::getPostPhysics(NewFrameID time) const
{
    ::boost::unordered_map<NewFrameID, TimeObjectListList>::const_iterator it(departures.find(time));
    if (it != departures.end()) {
        return it->second.getFlattenedVersion();
    }
    else {
        return ObjectList();
    }
}

ObjectList TimelineState::getPrePhysics(NewFrameID time) const
{
    ::boost::unordered_map<NewFrameID, TimeObjectListList>::const_iterator it(arrivals.find(time));
    if (it != arrivals.end()) {
        return it->second.getFlattenedVersion();
    }
    else {
        return ObjectList();
    }
}
