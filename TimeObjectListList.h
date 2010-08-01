#ifndef HG_TIME_OBJECT_LIST_LIST_H
#define HG_TIME_OBJECT_LIST_LIST_H

#include "ObjectList.h"
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr.hpp>
#include <vector>
namespace hg {
    
class TimeObjectListList
{
public:
	void setObjectList(int time, boost::shared_ptr<hg::ObjectList> newObjectList);
	void clearTime(int time);

	boost::shared_ptr<hg::ObjectList> getObjectListForManipulation(int time);

	void sortObjectLists();

	void sort();

	struct TimeObjectList 
	{
		int time;
		boost::shared_ptr<hg::ObjectList> objects;
	};



	static bool compareElements(boost::shared_ptr<TimeObjectList> first, boost::shared_ptr<TimeObjectList> second);

    //Should make this able to be const
	bool equals(hg::TimeObjectListList& other);
    std::vector<boost::shared_ptr<TimeObjectList> > list;
};
}
#endif //HG_TIME_OBJECT_LIST_LIST_H