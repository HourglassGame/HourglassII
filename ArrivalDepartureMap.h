#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H
namespace hg {
class ArrivalDepartureMap;
}

#include "ObjectList.h"
#include "TimeObjectListList.h"
#include <vector>
namespace hg {
class ArrivalDepartureMap
{

public:
	ArrivalDepartureMap(int timeLength);
    
    void setPlayerArrivalDeparture(const hg::Guy& guy, int departureTime, int arrivalTime);
    
    std::vector<int> updateDeparturesFromTime(int time, const TimeObjectListList& newDeparture);

	ObjectList& permanentDepartureObjectList(int arrivalTime);

	ObjectList getArrivals(int time);

    bool operator==(const ArrivalDepartureMap& other) const;
    inline bool operator!=(const ArrivalDepartureMap& other) const {
        return !(*this==other);
    }
private:
	int permanentDepartureIndex;
    std::vector<TimeObjectListList> arrivals;
    std::vector<TimeObjectListList> departures;
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
