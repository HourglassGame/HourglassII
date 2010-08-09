#include "ArrivalDepartureMap.h"
#include <algorithm>
#include <iostream>
using namespace std;
using namespace hg;

ArrivalDepartureMap::ArrivalDepartureMap(int timeLength) :
arrivals(timeLength),
departures(timeLength),
permanentDepartureIndex(timeLength)
{
}

ObjectList& ArrivalDepartureMap::permanentDepartureObjectList(int arrivalTime)
{
	return arrivals[arrivalTime].getObjectListForManipulation(permanentDepartureIndex);
}

//returns which frames are changed
std::vector<int> ArrivalDepartureMap::updateDeparturesFromTime(const int time, const TimeObjectListList& newDeparture)
{
    std::vector<int> changedTimes;
    //departures[time].sortObjectLists();
    TimeObjectListList::ListType::const_iterator ni(newDeparture.list.begin());
    const TimeObjectListList::ListType::const_iterator nend(newDeparture.list.end());
    TimeObjectListList::ListType::const_iterator oi(departures[time].list.begin());
    const TimeObjectListList::ListType::const_iterator oend(departures[time].list.end());
    
    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if ((*ni).first < (*oi).first) {
                    arrivals[(*ni).first].insertObjectList(time, (*ni).second);
                    changedTimes.push_back((*ni).first);
                    ++ni;
                }
                else if ((*ni).first == (*oi).first) {
                    if ((*ni).second != (*oi).second) {
                        arrivals[(*ni).first].list.find(time)->second = (*ni).second;
                        changedTimes.push_back((*ni).first);
                    }
                    ++ni;
                    break;
                }
                else {
                    arrivals[(*oi).first].list.erase(time);
                    changedTimes.push_back((*oi).first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    arrivals[(*oi).first].list.erase(time);
                    changedTimes.push_back((*oi).first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        arrivals[(*ni).first].insertObjectList(time, (*ni).second);
        changedTimes.push_back((*ni).first);
        ++ni;
    }
end:
    departures[time] = newDeparture;
    return changedTimes;
}

ObjectList ArrivalDepartureMap::getArrivals(int time)
{
	ObjectList returnList;

	for (TimeObjectListList::ListType::const_iterator it(arrivals[time].list.begin()), end(arrivals[time].list.end());
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
    
    return std::equal(departures.begin(), departures.end(), other.departures.begin())
    && std::equal(arrivals.begin(), arrivals.end(), other.arrivals.begin());
}


