#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H
namespace hg {
class ArrivalDepartureMap;
}

#include "ObjectList.h"
#include "TimeObjectListList.h"
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr.hpp>
#include <vector>
namespace hg {
class ArrivalDepartureMap
{

public:
	ArrivalDepartureMap(int timeLength);

	void setArrivalDeparturePair(int arrivalTime, int departureTime, boost::shared_ptr<ObjectList> objects);
    std::vector<int> updateDeparturesFromTime(int time, boost::shared_ptr<TimeObjectListList> newDeparture);

	boost::shared_ptr<ObjectList> permanentDepartureObjectList(int arrivalTime);

	boost::shared_ptr<ObjectList> getArrivals(int time, int guyIgnoreIndex);

	bool equals(boost::shared_ptr<ArrivalDepartureMap> other);

private:
	int permanentDepartureIndex;
    std::vector<boost::shared_ptr<TimeObjectListList> > arrivals;
    std::vector<boost::shared_ptr<TimeObjectListList> > departures;
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
