#ifndef HG_TIME_OBJECT_LIST_LIST_H
#define HG_TIME_OBJECT_LIST_LIST_H

#include "ObjectList.h"
#include "FrameID.h"
#include <map>
namespace hg {
class TimeObjectListList
{
public:
    TimeObjectListList();
	void setObjectList(FrameID time, const ObjectList& newObjectList);
	void clearTime(FrameID time);

	ObjectList& getObjectListForManipulation(FrameID time);
    
	void sortObjectLists();
    void insertObjectList(FrameID time, const ObjectList& newObjectList);
    typedef ::std::map<FrameID,ObjectList> ListType;

	bool equals(const TimeObjectListList& other) const;
    bool operator==(const TimeObjectListList& other) const;
    bool operator!=(const TimeObjectListList& other) const;
    ListType list;
};
}
#endif //HG_TIME_OBJECT_LIST_LIST_H
