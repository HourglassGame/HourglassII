#include "TimeObjectListList.h"
using namespace ::hg;
ObjectList& TimeObjectListList::getObjectListForManipulation(int time)
{
    return list[time];
}

void TimeObjectListList::setObjectList(int time, const ObjectList& newObjectList)
{
    list[time] = newObjectList;
}

//Inserts given object list at given time - noop if an object list already exists at the given time
void TimeObjectListList::insertObjectList(int time, const ObjectList& newObjectList)
{
    list.insert(ListType::value_type(time,newObjectList));
}

void TimeObjectListList::clearTime(int time)
{
    list.erase(time);
}

bool TimeObjectListList::operator==(const hg::TimeObjectListList& other) const
{
    return equals(other);
}

bool TimeObjectListList::operator!=(const hg::TimeObjectListList& other) const
{
    return !equals(other);
}

bool TimeObjectListList::equals(const TimeObjectListList& other) const
{
	if (list.size() != other.list.size())
	{
		return false;
	}
    return ::std::equal(list.begin(),list.end(),other.list.begin());
}

void TimeObjectListList::sortObjectLists()
{
	for (ListType::iterator it(list.begin()), end(list.end()); it != end; ++it)
	{
		(*it).second.sortElements();
	}
}