#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "RemoteDepartureEdit.h"

#include <boost/fusion/container/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/fusion/mpl.hpp>

#include <vector>

namespace hg {
template<typename T>
struct vector_of {
    typedef ::std::vector<T> type;
};
namespace object_list_detail {
    typedef ::boost::fusion::vector
        <
            Guy,
            Box,
            Button,
            Platform,
            Portal,
            RemoteDepartureEdit<Box>,//theif
            RemoteDepartureEdit<Box>,//extra
            RemoteDepartureEdit<Guy> //extra
        > ObjectListTypes;
    enum ElementID {
        guyList = 0,
        boxList = 1,
        buttonList = 2,
        platformList = 3,
        portalList = 4,
        boxThiefList = 5,
        boxExtraList = 6,
        guyExtraList = 7
    };
}
// Object list stores all data sent between frames or to rendering engine
class ObjectList
{
public:
    ObjectList();

    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);

    const ::std::vector<Guy>& getGuyListRef() const;
    const ::std::vector<Box>& getBoxListRef() const;
    const ::std::vector<Button>& getButtonListRef() const;
    const ::std::vector<Platform>& getPlatformListRef() const;
    const ::std::vector<Portal>& getPortalListRef() const;
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxThiefListRef() const;
    const ::std::vector<RemoteDepartureEdit<Box> >& getBoxExtraListRef() const;
    const ::std::vector<RemoteDepartureEdit<Guy> >& getGuyExtraListRef() const;
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
    //MUST CALL this to make lists sorted (required for operator==)
    void sort();
    
    void swap(ObjectList& other);

    bool operator==(const ObjectList& other) const;
    bool operator!=(const ObjectList& other) const;
	bool isEmpty() const;
private:
    friend class ObjectPtrList;
#ifndef NDEBUG
    bool sorted;
#endif //NDEBUG
    typedef ::boost::mpl::transform<object_list_detail::ObjectListTypes, vector_of< ::boost::mpl::_1> >::type ObjectListType;
    ObjectListType objectList_;
};
void swap(ObjectList& l, ObjectList& r);
class ObjectPtrList
{
public:
    ObjectPtrList();

    ObjectPtrList(const ObjectPtrList& other);
    ObjectPtrList& operator=(const ObjectPtrList& other);

    const ::std::vector<const Guy*>& getGuyListRef() const;
    const ::std::vector<const Box*>& getBoxListRef() const;
    const ::std::vector<const Button*>& getButtonListRef() const;
    const ::std::vector<const Platform*>& getPlatformListRef() const;
    const ::std::vector<const Portal*>& getPortalListRef() const;
    const ::std::vector<const RemoteDepartureEdit<Box>* >& getBoxThiefListRef() const;
    const ::std::vector<const RemoteDepartureEdit<Box>* >& getBoxExtraListRef() const;
    const ::std::vector<const RemoteDepartureEdit<Guy>* >& getGuyExtraListRef() const;
    //Add other ref getters as needed

    void add(const ObjectList& other);
    
    //MUST CALL this to make lists sorted (required for deterministic physics)
    void sort();
    
    void swap(ObjectPtrList& other);

    //bool operator==(const ObjectPtrList& other) const;
    //bool operator!=(const ObjectPtrList& other) const;
private:
    typedef boost::mpl::transform< object_list_detail::ObjectListTypes,vector_of<boost::add_pointer<boost::add_const<boost::mpl::_1> > > >::type ObjectPtrListType;
    ObjectPtrListType objectPtrList_;
};
void swap(ObjectPtrList& l, ObjectPtrList& r);
}
#endif //HG_OBJECT_LIST_H
