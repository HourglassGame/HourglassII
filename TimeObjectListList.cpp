#include "TimeObjectListList.h"
using namespace hg;
 boost::shared_ptr<ObjectList> TimeObjectListList::getObjectListForManipulation(int time)
{
	for (unsigned int i = 0; i < list.size(); ++i)
	{
		if (list[i]->time == time)
		{
			return list[i]->objects;
		}
	}

	list.push_back(boost::shared_ptr<TimeObjectList>(new TimeObjectList()));
	list.back()->time = time;
	list.back()->objects = boost::shared_ptr<ObjectList>(new ObjectList());
	return (list.back()->objects);
}

void TimeObjectListList::setObjectList(int time, boost::shared_ptr<ObjectList> newObjectList)
{
	for (unsigned int i = 0; i < list.size(); ++i)
	{
		if (list[i]->time == time)
		{
			list[i]->objects = newObjectList;
			return;
		}
	}
	
	list.push_back(boost::shared_ptr<TimeObjectList>(new TimeObjectList()));
	list.back()->time = time;
	list.back()->objects = newObjectList;
}

void TimeObjectListList::clearTime(int time)
{
	for (unsigned int i = 0; i < list.size(); ++i)
	{
		if (list[i]->time == time)
		{
			list.erase(list.begin() + i);
			return;
		}
	}
}

bool TimeObjectListList::compareElements(boost::shared_ptr<TimeObjectList> first , boost::shared_ptr<TimeObjectList> second)
{
	return (first->time < second->time);
}


bool TimeObjectListList::equals(TimeObjectListList& other)
{
	if (list.size() != other.list.size())
	{
		return false;
	}

	sort();
	other.sort();

	for (unsigned int i = 0; i < list.size(); ++i)
	{
		if (list[i]->time == other.list[i]->time)
		{
			if (!(list[i]->objects->equals(*(other.list[i]->objects.get()))))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

void TimeObjectListList::sortObjectLists()
{
	for (unsigned int i = 0; i < list.size(); ++i)
	{
		list[i]->objects->sortElements();
	}
}

void TimeObjectListList::sort()
{
	std::sort(list.begin(), list.end(), TimeObjectListList::compareElements);
}