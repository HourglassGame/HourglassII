

#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#include <boost/smart_ptr.hpp>
#include <vector>

class TimeObjectListList
{

public:

	void setObjectList(int time, boost::shared_ptr<ObjectList> newObjectList);
	void clearTime(int time);

	boost::shared_ptr<ObjectList> getObjectListForManipulation(int time);

	void sortObjectLists();

	void sort();

	struct TimeObjectList 
	{
		int time;
		boost::shared_ptr<ObjectList> objects;
	};

	vector<boost::shared_ptr<TimeObjectList> > list;

	static bool compareElements(boost::shared_ptr<TimeObjectList> first, boost::shared_ptr<TimeObjectList> second);

	bool equals(boost::shared_ptr<TimeObjectListList> other);

private:

	

};

