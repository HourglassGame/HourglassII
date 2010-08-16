#ifndef HG_TIME_OBJECT_LIST_LIST_H
#define HG_TIME_OBJECT_LIST_LIST_H

#include "ObjectList.h"
#include <boost/functional/hash.hpp>
#include <map>
namespace hg {
class TimeObjectListList
{
public:
    TimeObjectListList();
	void setObjectList(unsigned int time, const ObjectList& newObjectList);
	void clearTime(unsigned int time);

	ObjectList& getObjectListForManipulation(unsigned int time);
    
	void sortObjectLists();
    void insertObjectList(unsigned int time, const ObjectList& newObjectList);
    typedef std::map<unsigned int,ObjectList> ListType;

	bool equals(const TimeObjectListList& other) const;
    bool operator==(const TimeObjectListList& other) const;
    bool operator!=(const TimeObjectListList& other) const;
    ListType list;
    friend ::std::size_t hash_value(const TimeObjectListList& toHash);
};
::std::size_t hash_value(const TimeObjectListList& toHash);
}
#endif //HG_TIME_OBJECT_LIST_LIST_H
