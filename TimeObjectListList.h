#ifndef HG_TIME_OBJECT_LIST_LIST_H
#define HG_TIME_OBJECT_LIST_LIST_H

#include "ObjectList.h"
#include <map>
namespace hg {
    
class TimeObjectListList
{
public:
	void setObjectList(int time, const hg::ObjectList& newObjectList);
	void clearTime(int time);

	hg::ObjectList& getObjectListForManipulation(int time);
    
	void sortObjectLists();
    void insertObjectList(int time, const ObjectList& newObjectList);
    typedef std::map<int,ObjectList> ListType;

    //Should make this able to be const
	bool equals(const hg::TimeObjectListList& other) const;
    bool operator==(const hg::TimeObjectListList& other) const;
    bool operator!=(const hg::TimeObjectListList& other) const;
    ListType list;
};
}
#endif //HG_TIME_OBJECT_LIST_LIST_H