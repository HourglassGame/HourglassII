#include "TimelineState.h"
#include <cassert>
#include <algorithm>
#include <iostream>
#include "FrameUpdateSet.h"

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

FrameUpdateSet TimelineState::updateWithNewDepartures(const map<FrameID, TimeObjectListList>& newDepartures)
{
    FrameUpdateSet newWaveFrames;
    for(map<FrameID, TimeObjectListList>::const_iterator
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

ObjectList& TimelineState::permanentDepartureObjectList(unsigned int arrivalTime)
{
	return arrivals.at(arrivalTime).getObjectListForManipulation(permanentDepartureIndex);
}

//returns which frames are changed
FrameUpdateSet TimelineState::updateDeparturesFromTime(const FrameID time, const TimeObjectListList& newDeparture)
{
    FrameUpdateSet changedTimes;

    Iterator ni(newDeparture.list.begin());
    const Iterator nend(newDeparture.list.end());
    Iterator oi(departures.at(time).list.begin());
    const Iterator oend(departures.at(time).list.end());

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
                        arrivals.at((*ni).first).list.find(time)->second = (*ni).second;
                        changedTimes.addFrame((*ni).first);
                    }
                    ++ni;
                    break;
                }
                else {
                    arrivals.at((*oi).first).list.erase(time);
                    changedTimes.addFrame((*oi).first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    arrivals.at((*oi).first).list.erase(time);
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
    ObjectList returnList;

	for (Iterator it(departures.at(time).list.begin()), end(departures.at(time).list.end());
         it != end; ++it)
	{
		returnList.add(it->second);
	}
    returnList.sortElements();
	return returnList;
}
//Assertion failure is here.
ObjectList TimelineState::getPrePhysics(unsigned int time) const
{
	ObjectList returnList;

	for (Iterator it(arrivals.at(time).list.begin()), end(arrivals.at(time).list.end());
         it != end; ++it)
	{
		returnList.add(it->second);
	}

    returnList.sortElements();
	return returnList;
}

bool TimelineState::operator==(const TimelineState& other) const
{
	if (permanentDepartureIndex != other.permanentDepartureIndex)
	{
		return false;
	}

    assert(departures.size()==other.departures.size());
    assert(arrivals.size()==other.arrivals.size());

    return equal(departures.begin(), departures.end(), other.departures.begin())
    && equal(arrivals.begin(), arrivals.end(), other.arrivals.begin());
}
