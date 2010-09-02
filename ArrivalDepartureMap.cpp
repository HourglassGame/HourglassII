#include "ArrivalDepartureMap.h"
#include <cassert>
#include <algorithm>
#include <iostream>

using namespace ::std;
using namespace ::hg;

typedef TimeObjectListList::ListType::const_iterator Iterator;

ArrivalDepartureMap::ArrivalDepartureMap(unsigned int timeLength) :
permanentDepartureIndex(timeLength),
arrivals(timeLength),
departures(timeLength)
{
}

ObjectList ArrivalDepartureMap::Frame::getPrePhysics() const
{
    return this_.getPrePhysics(time_);
}
FrameID ArrivalDepartureMap::Frame::getTime() const
{
    return time_;
}

ArrivalDepartureMap::Frame::Frame(const ArrivalDepartureMap& mapPtr, FrameID time) :
time_(time),
this_(mapPtr)
{
}

vector<FrameID> ArrivalDepartureMap::updateWithNewDepartures(const map<FrameID, TimeObjectListList>& newDepartures)
{
    vector<FrameID> newWaveFrames;
    for(map<FrameID, TimeObjectListList>::const_iterator
            it(newDepartures.begin()), end(newDepartures.end());
            it != end;
            ++it)
    {
        vector<FrameID> temp (updateDeparturesFromTime(it->first, it->second));
        newWaveFrames.insert(newWaveFrames.end(),temp.begin(), temp.end());
    }
    sort(newWaveFrames.begin(), newWaveFrames.end());
    newWaveFrames.erase(unique(newWaveFrames.begin(), newWaveFrames.end()), newWaveFrames.end());
    return newWaveFrames;
}

ArrivalDepartureMap::Frame ArrivalDepartureMap::getFrame(FrameID whichFrame) const
{
    return Frame(*this, whichFrame);
}

ObjectList& ArrivalDepartureMap::permanentDepartureObjectList(unsigned int arrivalTime)
{
	return arrivals.at(arrivalTime).getObjectListForManipulation(permanentDepartureIndex);
}

//returns which frames are changed
vector<FrameID> ArrivalDepartureMap::updateDeparturesFromTime(const FrameID time, const TimeObjectListList& newDeparture)
{
    vector<FrameID> changedTimes;

    Iterator ni(newDeparture.list.begin());
    const Iterator nend(newDeparture.list.end());
    Iterator oi(departures.at(time).list.begin());
    const Iterator oend(departures.at(time).list.end());

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if ((*ni).first < (*oi).first) {
                    arrivals.at((*ni).first).insertObjectList(time, (*ni).second);
                    changedTimes.push_back((*ni).first);
                    ++ni;
                }
                else if ((*ni).first == (*oi).first) {
                    if ((*ni).second != (*oi).second) {
                        arrivals.at((*ni).first).list.find(time)->second = (*ni).second;
                        changedTimes.push_back((*ni).first);
                    }
                    ++ni;
                    break;
                }
                else {
                    arrivals.at((*oi).first).list.erase(time);
                    changedTimes.push_back((*oi).first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    arrivals.at((*oi).first).list.erase(time);
                    changedTimes.push_back((*oi).first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        arrivals.at((*ni).first).insertObjectList(time, (*ni).second);
        changedTimes.push_back((*ni).first);
        ++ni;
    }
end:
    departures.at(time) = newDeparture;
    return changedTimes;
}

ObjectList ArrivalDepartureMap::getPostPhysics(FrameID time) const
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

ObjectList ArrivalDepartureMap::getPrePhysics(unsigned int time) const
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

bool ArrivalDepartureMap::operator==(const ArrivalDepartureMap& other) const
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
