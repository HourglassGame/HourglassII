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
    
	void setObjectList(const FrameID& time, const ObjectList& newObjectList);
	void clearTime(FrameID time);
    
    void insertObjectList(const FrameID& time, const ObjectList& newObjectList);
    
    //puts into time a new Object list which contains all the objects in newObjectList
    //and all the objects already in the list at that time
    void addObjectList(const FrameID& time, const ObjectList& newObjectList);
    
    bool operator==(const TimeObjectListList& other) const;
    bool operator!=(const TimeObjectListList& other) const;
    
    ObjectList getFlattenedVersion() const;
    ObjectList getFlattenedVersion(const FrameID& time, const PauseInitiatorID& whichPrePause) const;
    
    
    typedef ::std::map<FrameID,ObjectList> ListType;
    
    //Big and potentially very painful privacy leak here, please fix.
    typedef ListType::const_iterator const_iterator;
    
    const_iterator begin() const
    {
        return list_.begin();
    }
    const_iterator end() const
    {
        return list_.end();
    }
    
private:
    friend void swap(TimeObjectListList& l, TimeObjectListList& r);
    ListType list_;
};
void swap(TimeObjectListList& l, TimeObjectListList& r);
}
#endif //HG_TIME_OBJECT_LIST_LIST_H
