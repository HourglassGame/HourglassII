#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "TriggerData.h"
#include "RemoteDepartureEdit_def.h"

#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/fusion/mpl.hpp>
#include <boost/fusion/include/find.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range.hpp>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_pointer.hpp>

#include <vector>

namespace hg {
template<typename T>
struct vector_of {
    typedef std::vector<T> type;
};
namespace object_list_detail {
typedef boost::fusion::vector
<
Guy,
Box,
Button,
Platform,
Portal,
TriggerData,
RemoteDepartureEdit<Thief,Box>,
RemoteDepartureEdit<Extra,Box>,
RemoteDepartureEdit<Extra,Guy>
> ObjectListTypes;
}
// Object list stores all data sent between frames or to rendering engine
class ObjectList
{
public:
    ObjectList();

    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);
    
    template<typename ObjectT>
    typename vector_of<ObjectT>::type const& getList() const;
    
    template<typename ObjectT>
    void add(const ObjectT& toCopy);
    
    template<typename ObjectRangeT>
    void addRange(const ObjectRangeT& toAdd);

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
    typedef boost::mpl::transform<object_list_detail::ObjectListTypes, vector_of<boost::mpl::_1> >::type ObjectListType;
    ObjectListType objectList_;
};
void swap(ObjectList& l, ObjectList& r);

template<typename ObjectT>
void ObjectList::add(const ObjectT& toCopy)
{
    boost::fusion::deref(
        boost::fusion::find<
            typename vector_of<ObjectT>::type
        >(objectList_)
    ).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

template<typename ObjectRangeT>
void ObjectList::addRange(const ObjectRangeT& toAdd)
{
    boost::push_back(
        boost::fusion::deref(
            boost::fusion::find<
                typename vector_of<typename boost::range_value<ObjectRangeT>::type >::type
            >(objectList_)),
        toAdd);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

//Be very careful about using this on unsorted lists.
//REALLY! BE CAREFUL. You could end up with non-deterministic/unstable stuff 
//if you have code which depends on the order of the resulting range.
template<typename ObjectT>
typename vector_of<ObjectT>::type const& ObjectList::getList() const
{
    return boost::fusion::deref(
        boost::fusion::find<
            typename vector_of<ObjectT>::type
        >(objectList_));
}

class ObjectPtrList
{
public:
    ObjectPtrList();

    ObjectPtrList(const ObjectPtrList& other);
    ObjectPtrList& operator=(const ObjectPtrList& other);

    template<typename ObjectT>
    boost::indirected_range<const typename vector_of<const ObjectT*>::type > getList() const;

    void add(const ObjectList& other);

    //MUST CALL this to make lists sorted (required for deterministic physics)
    void sort();

    void swap(ObjectPtrList& other);

private:
    typedef
    boost::mpl::transform<
        object_list_detail::ObjectListTypes,
        vector_of<
            boost::add_pointer<
                boost::add_const<boost::mpl::_1> > > >::type ObjectPtrListType;
    ObjectPtrListType objectPtrList_;
};
void swap(ObjectPtrList& l, ObjectPtrList& r);
template<typename ObjectT>
boost::indirected_range<const typename vector_of<const ObjectT*>::type > ObjectPtrList::getList() const
{
    return boost::adaptors::indirect(
        boost::fusion::deref(
            boost::fusion::find<
                typename vector_of<const ObjectT*>::type
            >(objectPtrList_)));
}
}
#endif //HG_OBJECT_LIST_H
