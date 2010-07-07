
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#ifndef INC_TIMEOBJECTLISTLIST
#define INC_TIMEOBJECTLISTLIST
#include "TimeObjectListList.h"
#endif // INC_TIMEOBJECTLISTLIST

#include <boost/smart_ptr.hpp>
#include <vector>


class ArrivalDepartureMap
{

public:
	ArrivalDepartureMap(int timeLength);

	void setArrivalDeparturePair(int arrivalTime, int departureTime, boost::shared_ptr<ObjectList> objects);
	vector<int> updateDeparturesFromTime(int time, boost::shared_ptr<TimeObjectListList> newDeparture);

	boost::shared_ptr<ObjectList> permanentDepartureObjectList(int arrivalTime);

	boost::shared_ptr<ObjectList> getArrivals(int time, int guyIgnoreIndex);

	bool equals(boost::shared_ptr<ArrivalDepartureMap> other);

	// these 3 only public for equals()
	int permanentDepartureIndex;
	vector<boost::shared_ptr<TimeObjectListList> > arrivals;
	vector<boost::shared_ptr<TimeObjectListList> > departures;

private:

};

