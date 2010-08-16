#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H

//#include "ObjectList.h"
#include "TimeObjectListList.h"
#include <boost/functional/hash.hpp>
#include <vector>

namespace hg {
class ArrivalDepartureMap
{
public:
	ArrivalDepartureMap(unsigned int timeLength);
    
    ::std::vector<unsigned int> updateDeparturesFromTime(unsigned int time, const TimeObjectListList& newDeparture);

	ObjectList& permanentDepartureObjectList(unsigned int arrivalTime);

	ObjectList getArrivals(unsigned int time);

    bool operator==(const ArrivalDepartureMap& other) const;
    inline bool operator!=(const ArrivalDepartureMap& other) const {
        return !(*this==other);
    }
private:
    friend ::std::size_t hash_value(const ArrivalDepartureMap& toHash);
	unsigned int permanentDepartureIndex;
    ::std::vector<TimeObjectListList> arrivals;
    ::std::vector<TimeObjectListList> departures;
};
    ::std::size_t hash_value(const ArrivalDepartureMap& toHash);
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
