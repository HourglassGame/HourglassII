#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "RemoteDepartureEdit.cpp"
#include "TimeDirection.h"

#include <boost/shared_ptr.hpp>

#include <vector>

namespace hg {
struct ObjectListData;
class ObjectList;
class MutableObjectList;
// Object list stores all data sent between frames or to rendering engine
class ObjectList
{
public:
    ObjectList();

    ~ObjectList();
    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);

    ObjectList(const MutableObjectList& other);
    ObjectList& operator=(const MutableObjectList& other);

    const ::std::vector<Guy>& getGuyListRef() const;
    const ::std::vector<Box>& getBoxListRef() const;
    const ::std::vector<Button>& getButtonListRef() const;
    const ::std::vector<Platform>& getPlatformListRef() const;
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxThiefListRef() const;
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxExtraListRef() const;
    const ::std::vector<RemoteDepartureEdit<Guy> >& getGuyExtraListRef() const;
    //Add other ref getters as needed

    bool operator==(const ObjectList& other) const;
    bool operator!=(const ObjectList& other) const;
	bool isEmpty() const;
private:
    friend class MutableObjectList;
    ::boost::shared_ptr<ObjectListData> data_;
};

class MutableObjectList {
public:
    MutableObjectList();
    ~MutableObjectList();
    MutableObjectList(const ObjectList& other);
    MutableObjectList(const MutableObjectList& other);
    MutableObjectList& operator=(const MutableObjectList& other);

	void add(const Guy& toCopy);
	void add(const Box& toCopy);
	void add(const Button& toCopy);
	void add(const Platform& toCopy);
	void addThief(const RemoteDepartureEdit<Box>& toCopy);
	void addExtra(const RemoteDepartureEdit<Box>& toCopy);
	void addExtra(const RemoteDepartureEdit<Guy>& toCopy);

    void add(const MutableObjectList& other);
    void add(const ObjectList& other);
private:
    friend class ObjectList;
    void makeUnique();
    ::boost::shared_ptr<ObjectListData> data_;
};
}
#endif //HG_OBJECT_LIST_H
