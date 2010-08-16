#include "ArrivalDepartureMap.h"
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

ObjectList& ArrivalDepartureMap::permanentDepartureObjectList(unsigned int arrivalTime)
{
	return arrivals.at(arrivalTime).getObjectListForManipulation(permanentDepartureIndex);
}

//returns which frames are changed
vector<unsigned int> ArrivalDepartureMap::updateDeparturesFromTime(const unsigned int time, const TimeObjectListList& newDeparture)
{
    vector<unsigned int> changedTimes;
    
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

ObjectList ArrivalDepartureMap::getArrivals(unsigned int time)
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
::std::size_t hg::hash_value(const ArrivalDepartureMap& toHash)
{
    //PermanentDepartureIndex should be the same throughout the execution of a level, so not hashed.
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.arrivals);
    ::boost::hash_combine(seed, toHash.departures);
    return seed;
}

