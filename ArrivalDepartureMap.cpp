#include "ArrivalDepartureMap.h"

using namespace hg;

ArrivalDepartureMap::ArrivalDepartureMap(int timeLength)
{
	arrivals.reserve(timeLength);
	departures.reserve(timeLength);

	int permanentDepartureIndex = timeLength;
	// (timeLength+1) for permanent departures

	for (int i = 0; i < timeLength; ++i)
    {
		arrivals.push_back(boost::shared_ptr<TimeObjectListList>(new TimeObjectListList()));
		departures.push_back(boost::shared_ptr<TimeObjectListList>(new TimeObjectListList()));
	}
}

void ArrivalDepartureMap::setArrivalDeparturePair(int arrivalTime, int departureTime, boost::shared_ptr<ObjectList> objects)
{
	if (objects->isEmpty())
	{
		arrivals[arrivalTime]->clearTime(departureTime);
		departures[departureTime]->clearTime(arrivalTime);
	}
	else
	{
		arrivals[arrivalTime]->setObjectList(departureTime, objects);
		departures[departureTime]->setObjectList(arrivalTime, objects);
	}

}

boost::shared_ptr<ObjectList> ArrivalDepartureMap::permanentDepartureObjectList(int arrivalTime)
{
	return arrivals[arrivalTime]->getObjectListForManipulation(permanentDepartureIndex);
}

// returns which frames are changed
std::vector<int> ArrivalDepartureMap::updateDeparturesFromTime(int time, boost::shared_ptr<TimeObjectListList> newDeparture)
{
	newDeparture->sort();
	departures[time]->sort();

	unsigned int oi = 0;
	unsigned int ni = 0;

	if (oi >= departures[time]->list.size()) 
	{
		oi = -1;
	}
	if (ni >= newDeparture->list.size()) 
	{
		ni = -1;
	}

    std::vector<int> changedTimes; // times that have been changed so require recalculation in TimeEngine

	// check which times have changed
	while(true)
	{
		if (oi == -1)
		{
			if (ni == -1)
			{
				break; // -1 indicates the end of the vector has been reached
			}
			else
			{
				// no more old departures so all remaining times are added so different
				changedTimes.push_back(newDeparture->list[ni]->time); 
				setArrivalDeparturePair(newDeparture->list[ni]->time, time, newDeparture->list[ni]->objects);
				++ni;
				if (ni >= newDeparture->list.size()) 
				{
					ni = -1;
				}
			}
		}
		else
		{
			if (ni == -1)
			{
				// no more new departures so all remaining times will be erased
				changedTimes.push_back(departures[time]->list[oi]->time);
				setArrivalDeparturePair(departures[time]->list[oi]->time, time, boost::shared_ptr<ObjectList>(new ObjectList()));
				++oi;
				if (oi >= departures[time]->list.size()) 
				{
					oi = -1;
				}
			}
			else
			{
				if (newDeparture->list[ni]->time == departures[time]->list[oi]->time) // departure times are equal
				{
					if (!(newDeparture->list[ni]->objects->equals( *(departures[time]->list[oi]->objects.get()) )))
					{
						// if the object lists are not equal change time
						setArrivalDeparturePair(newDeparture->list[ni]->time, time, newDeparture->list[ni]->objects);
						changedTimes.push_back(newDeparture->list[ni]->time);
					}
					++oi;
					if (oi >= departures[time]->list.size()) 
					{
						oi = -1;
					}
					++ni;
					if (ni >= newDeparture->list.size()) 
					{
						ni = -1;
					}
				}
				else if (newDeparture->list[ni]->time > departures[time]->list[oi]->time) // new departure occurs AFTER old departure
				{
					changedTimes.push_back(departures[time]->list[oi]->time);
					setArrivalDeparturePair(departures[time]->list[oi]->time, time, boost::shared_ptr<ObjectList>(new ObjectList()));
					++oi;
					if (oi >= departures[time]->list.size())
					{
						oi = -1;
					}
				}
				else // new departure occurs BEFORE old departure
				{
					changedTimes.push_back(newDeparture->list[ni]->time);
					setArrivalDeparturePair(newDeparture->list[ni]->time, time, newDeparture->list[ni]->objects);
					++ni;
					if (ni >= newDeparture->list.size()) 
					{
						ni = -1;
					}
				}
			}
		}
	}

	// overwrite old departures
	departures[time] = newDeparture;

	return changedTimes;
}

boost::shared_ptr<ObjectList> ArrivalDepartureMap::getArrivals(int time, int guyIgnoreIndex)
{
	boost::shared_ptr<ObjectList> returnList = boost::shared_ptr<ObjectList>(new ObjectList());

	for (unsigned int i = 0; i < arrivals[time]->list.size(); ++i)
	{
		returnList->add(*(arrivals[time]->list[i]->objects.get()), guyIgnoreIndex);
	}

	return returnList;
}


bool ArrivalDepartureMap::equals(boost::shared_ptr<ArrivalDepartureMap> other)
{
	if (permanentDepartureIndex != other->permanentDepartureIndex)
	{
		return false;
	}
	
	for (unsigned int i = 0; i < departures.size(); ++i)
	{
		if (!(departures[i]->equals(*other->departures[i])))
		{
			return false;
		}
	}

	for (unsigned int i = 0; i < arrivals.size(); ++i)
	{
		if (!(arrivals[i]->equals(*other->arrivals[i])))
		{
			return false;
		}
	}

	return true;

}