#include "TimeObjectListList.h"
using namespace ::hg;

TimeObjectListList::TimeObjectListList() :
list_()
{
}

void TimeObjectListList::setObjectList(const NewFrameID& time, const ObjectList& newObjectList)
{
    list_[time] = newObjectList;
}

//Inserts given object list at given time - noop if an object list already exists at the given time
void TimeObjectListList::insertObjectList(const NewFrameID& time, const ObjectList& newObjectList)
{
    list_.insert(ListType::value_type(time,newObjectList));
}

void TimeObjectListList::clearTime(NewFrameID time)
{
    list_.erase(time);
}

bool TimeObjectListList::operator==(const TimeObjectListList& other) const
{
    return list_ == other.list_;
}

bool TimeObjectListList::operator!=(const TimeObjectListList& other) const
{
    return !(*this == other);
}

ObjectList TimeObjectListList::getFlattenedVersion() const
{
    MutableObjectList returnList;
    
	for (ListType::const_iterator it(list_.begin()), eend(list_.end()); it != eend; ++it)
	{
		returnList.add(it->second);
	}
    
	return ObjectList(returnList);
}
