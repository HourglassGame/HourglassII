#if 0
#include "TimeObjectListList.h"
#include "ObjectList.h"
namespace hg {
TimeObjectListList::TimeObjectListList() :
list_()
{
}

void TimeObjectListList::setObjectList(const FrameID& time, const ObjectList& newObjectList)
{
    list_[time] = newObjectList;
}

//Inserts given object list at given time - noop if an object list already exists at the given time
void TimeObjectListList::insertObjectList(const FrameID& time, const ObjectList& newObjectList)
{
    list_.insert(ListType::value_type(time,newObjectList));
}

void TimeObjectListList::addObjectList(const FrameID& time, const ObjectList& newObjectList)
{
    MutableObjectList sum(newObjectList);
    sum.add(list_[time]);
    list_[time] = ObjectList(sum);
}

void TimeObjectListList::clearTime(FrameID time)
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
ObjectList TimeObjectListList::getFlattenedVersion(const FrameID& time, const PauseInitiatorID& whichPrePause) const
{
    size_t decisionDepth(time.universe().pauseDepth() + 1);
    MutableObjectList returnList;
    if (whichPrePause.type_==pauseinitiatortype::INVALID) {
        for (ListType::const_iterator it(list_.begin()), eend(list_.end()); it != eend; ++it)
        {
            if (it->first.universe().pauseDepth() < decisionDepth) {
                returnList.add(it->second);
            }
        }
    }
    else {
      	for (ListType::const_iterator it(list_.begin()), eend(list_.end()); it != eend; ++it)
        {
            if (it->first.universe().pauseDepth() == decisionDepth && it->first.universe().initiatorID() == whichPrePause) {
                returnList.add(it->second);
            }
        }  
    }

	return ObjectList(returnList);
}
void swap(TimeObjectListList& l, TimeObjectListList& r)
{
    swap(l.list_, r.list_);
}

}//namespace hg
#endif
