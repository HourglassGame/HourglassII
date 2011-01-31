#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "RemoteDepartureEdit.cpp"
#include "TimeDirection.h"

#include <vector>

namespace hg {
// Object list stores all data sent between frames or to rendering engine
class ObjectList
{
public:
    ObjectList();

    ~ObjectList();
    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);

    const ::std::vector<Guy>& getGuyListRef() const {return guyList;}
    const ::std::vector<Box>& getBoxListRef() const {return boxList;}
    const ::std::vector<Button>& getButtonListRef() const {return buttonList;}
    const ::std::vector<Platform>& getPlatformListRef() const {return platformList;}
    const ::std::vector<Portal>& getPortalListRef() const {return portalList;}
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxThiefListRef() const {return boxThiefList;}
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxExtraListRef() const {return boxExtraList;}
    const ::std::vector<RemoteDepartureEdit<Guy> >& getGuyExtraListRef() const {return guyExtraList;}
    //Add other ref getters as needed

	void add(const Guy& toCopy);
	void add(const Box& toCopy);
	void add(const Button& toCopy);
	void add(const Platform& toCopy);
	void add(const Portal& toCopy);
	void addThief(const RemoteDepartureEdit<Box>& toCopy);
	void addExtra(const RemoteDepartureEdit<Box>& toCopy);
	void addExtra(const RemoteDepartureEdit<Guy>& toCopy);
    
    void add(const ObjectList& other);
    //MUST CALL THIS after constructing complete ObjectList!
    void sort();
    
    void swap(ObjectList& other);

    bool operator==(const ObjectList& other) const;
    bool operator!=(const ObjectList& other) const;
	bool isEmpty() const;
private:
    friend class ObjectPtrList;
    ::std::vector<Guy> guyList;
    ::std::vector<Box> boxList;
    ::std::vector<Button> buttonList;
    ::std::vector<Platform> platformList;
    ::std::vector<Portal> portalList;
    ::std::vector<RemoteDepartureEdit<Box> > boxThiefList;
    ::std::vector<RemoteDepartureEdit<Box> > boxExtraList;
    ::std::vector<RemoteDepartureEdit<Guy> > guyExtraList;
};
void swap(ObjectList& l, ObjectList& r);
class ObjectPtrList
{
public:
    ObjectPtrList();

    ~ObjectPtrList();
    ObjectPtrList(const ObjectPtrList& other);
    ObjectPtrList& operator=(const ObjectPtrList& other);

    const ::std::vector<const Guy*>& getGuyListRef() const {return guyList;}
    const ::std::vector<const Box*>& getBoxListRef() const {return boxList;}
    const ::std::vector<const Button*>& getButtonListRef() const {return buttonList;}
    const ::std::vector<const Platform*>& getPlatformListRef() const {return platformList;}
    const ::std::vector<const Portal*>& getPortalListRef() const {return portalList;}
    const ::std::vector<const RemoteDepartureEdit<Box>* >& getBoxThiefListRef() const {return boxThiefList;}
    const ::std::vector<const RemoteDepartureEdit<Box>* >& getBoxExtraListRef() const {return boxExtraList;}
    const ::std::vector<const RemoteDepartureEdit<Guy>* >& getGuyExtraListRef() const {return guyExtraList;}
    //Add other ref getters as needed

    void add(const ObjectList& other);
    
    //MUST CALL THIS after constructing complete ObjectPtrList!
    void sort();
    
    void swap(ObjectPtrList& other);

    //bool operator==(const ObjectPtrList& other) const;
    //bool operator!=(const ObjectPtrList& other) const;
private:
    ::std::vector<const Guy*> guyList;
    ::std::vector<const Box*> boxList;
    ::std::vector<const Button*> buttonList;
    ::std::vector<const Platform*> platformList;
    ::std::vector<const Portal*> portalList;
    ::std::vector<const RemoteDepartureEdit<Box>*> boxThiefList;
    ::std::vector<const RemoteDepartureEdit<Box>*> boxExtraList;
    ::std::vector<const RemoteDepartureEdit<Guy>*> guyExtraList;
};
void swap(ObjectPtrList& l, ObjectPtrList& r);
}
#endif //HG_OBJECT_LIST_H
