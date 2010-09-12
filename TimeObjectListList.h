#ifndef HG_TIME_OBJECT_LIST_LIST_H
#define HG_TIME_OBJECT_LIST_LIST_H

#include "ObjectList.h"
#include "SimpleFrameID.h"
#include <map>
namespace hg {
class TimeObjectListList
{
public:
    TimeObjectListList();
    
	void setObjectList(const SimpleFrameID& time, const ObjectList& newObjectList);
	void clearTime(SimpleFrameID time);
    
    void insertObjectList(const SimpleFrameID& time, const ObjectList& newObjectList);
    
    bool operator==(const TimeObjectListList& other) const;
    bool operator!=(const TimeObjectListList& other) const;
    
    ObjectList getFlattenedVersion() const;
    
    typedef ::std::map<SimpleFrameID,ObjectList> ListType;
    
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
    ListType list_;
};
}
#endif //HG_TIME_OBJECT_LIST_LIST_H
