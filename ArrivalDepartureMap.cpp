#include "ArrivalDepartureMap.h"

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
    TimeObjectListList::ListType::const_iterator oi(departures[time].list.begin());
    TimeObjectListList::ListType::const_iterator ni(newDeparture.list.begin());
    TimeObjectListList::ListType::const_iterator oend(departures[time].list.end());
    TimeObjectListList::ListType::const_iterator nend(newDeparture.list.end());
    
    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if ((*ni).first < (*oi).first) {
                    arrivals[(*ni).first].insertObjectList(time, (*ni).second);
                    changedTimes.push_back((*ni).first);
                    ++ni;
                }
                else if ((*ni).first == (*oi).first) {
                    arrivals[(*ni).first].list.find(time)->second = (*ni).second;
                    changedTimes.push_back((*ni).first);
                    ++ni;
                    goto bottom;
                }
                else {
                    arrivals[(*oi).first].list.erase(time);
                    changedTimes.push_back((*oi).first);
                    goto bottom;
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
    bottom:
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

ObjectList ArrivalDepartureMap::getArrivals(int time, int guyIgnoreIndex)
{
	ObjectList returnList;

	for (TimeObjectListList::ListType::const_iterator it(arrivals[time].list.begin()), end(arrivals[time].list.end());
         it != end; ++it)
	{
		returnList.add(it->second, guyIgnoreIndex);
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
	
	for (unsigned int i = 0; i < departures.size(); ++i)
	{
		if (departures[i] != other.departures[i])
		{
			return false;
		}
	}
    
	for (unsigned int i = 0; i < arrivals.size(); ++i)
	{
		if (arrivals[i] != other.arrivals[i])
		{
			return false;
		}
	}
    
	return true;
}


