#include "ObjectList.h"

#include "SortedByIndex.h"

#include <boost/utility/swap.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/next.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/zip_view.hpp>
#include <boost/fusion/container/vector/vector10.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <functional>
#include <iterator>

#include <cassert>

namespace {
template<typename T>
struct EqualIndices {
    bool operator()(const T& l, const T& r) const {
        return l.getIndex() == r.getIndex();
    }
};

template<typename T>
struct EqualIndices<T*> {
    bool operator()(T* l, T* r) const {
        return l->getIndex() == r->getIndex();
    }
};

template<typename ForwardRange>
bool containsNoElementsWithEqualIndices(const ForwardRange& range)
{
    return boost::adjacent_find<boost::return_found>
            (range,
             EqualIndices
             <
                typename std::iterator_traits
                <
                    typename boost::range_iterator<ForwardRange>::type
                >::value_type
             >())
            == boost::end(range);
}
template<class SequenceOfSequences, class Func>
void n_ary_for_each(SequenceOfSequences const & s, Func const & f)
{
    using namespace boost::fusion;
    for_each(zip_view<SequenceOfSequences>(s),
        fused_procedure<Func const &>(f));
}
} //namespace

namespace hg {
ObjectList::ObjectList() :
#ifndef NDEBUG
sorted(true),
#endif //NDEBUG
objectList_()
{
}

ObjectList::ObjectList(const ObjectList& other):
#ifndef NDEBUG
sorted(other.sorted),
#endif //NDEBUG
objectList_(other.objectList_)
{
}
ObjectList& ObjectList::operator=(const ObjectList& other)
{
#ifndef NDEBUG
    sorted = other.sorted;
#endif //NDEBUG
    objectList_ = other.objectList_;
    return *this;
}

const std::vector<Guy>& ObjectList::getGuyListRef() const 
{
    return boost::fusion::at_c<object_list_detail::guyList>(objectList_);
}
const std::vector<Box>& ObjectList::getBoxListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxList>(objectList_);
}
const std::vector<Button>& ObjectList::getButtonListRef() const
{
    return boost::fusion::at_c<object_list_detail::buttonList>(objectList_);
}
const std::vector<Platform>& ObjectList::getPlatformListRef() const
{
    return boost::fusion::at_c<object_list_detail::platformList>(objectList_);
}
const std::vector<Portal>& ObjectList::getPortalListRef() const
{
    return boost::fusion::at_c<object_list_detail::portalList>(objectList_);
}
const std::vector<RemoteDepartureEdit<Box> >& ObjectList::getBoxThiefListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxThiefList>(objectList_);
}
const std::vector<RemoteDepartureEdit<Box> >& ObjectList::getBoxExtraListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxExtraList>(objectList_);
}
const std::vector<RemoteDepartureEdit<Guy> >& ObjectList::getGuyExtraListRef() const
{
    return boost::fusion::at_c<object_list_detail::guyExtraList>(objectList_);
}

void ObjectList::add(const Guy& toCopy)
{
    boost::fusion::at_c<object_list_detail::guyList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Box& toCopy)
{
    boost::fusion::at_c<object_list_detail::boxList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Button& toCopy)
{
    boost::fusion::at_c<object_list_detail::buttonList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Platform& toCopy)
{
    boost::fusion::at_c<object_list_detail::platformList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Portal& toCopy)
{
    boost::fusion::at_c<object_list_detail::portalList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addThief(const RemoteDepartureEdit<Box>& toCopy)
{
    boost::fusion::at_c<object_list_detail::boxThiefList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addExtra(const RemoteDepartureEdit<Box>& toCopy)
{
    boost::fusion::at_c<object_list_detail::boxExtraList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addExtra(const RemoteDepartureEdit<Guy>& toCopy)
{
    boost::fusion::at_c<object_list_detail::guyExtraList>(objectList_).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

struct Insert
{
    template<typename Container, typename SinglePassRange>
    void operator()(Container& toInsertInto, const SinglePassRange& toInsert) const
    {
        boost::push_back(toInsertInto, toInsert);
    }
};

void ObjectList::add(const ObjectList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectList_, other.objectList_), Insert());
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
template<template<class> class Comparitor = std::less>
struct Sort
{
    template <typename ListType>
    void operator()(ListType& toSort, typename boost::disable_if<sorted_by_index<ListType> >::type* = 0) const
    {
        boost::sort(toSort, Comparitor<typename ListType::value_type>());
    }
    template <typename ListType>
    void operator()(ListType& toSort, typename boost::enable_if<sorted_by_index<ListType> >::type* = 0) const
    {
        boost::sort(toSort, Comparitor<typename ListType::value_type>());
        //These lists are sorted on index alone, but have equality based on all their members.
        //This means that they must never have more than one element with the same index, 
        //because that could cause equal ObjectLists to be found to be different because the order in which the lists within the
        //two ObjectLists are sorted is different, even though the lists contain the same elements.
        assert(containsNoElementsWithEqualIndices(toSort));
    }
};


//MUST CALL THIS before calling operator== constructing complete ObjectList!
void ObjectList::sort()
{
    boost::fusion::for_each(objectList_, Sort<>());
#ifndef NDEBUG
    sorted = true;
#endif //NDEBUG
}

struct Swap
{
    template<typename T1, typename T2>
    void operator()(T1& lhs, T2& rhs) const
    {
        boost::swap(lhs, rhs);
    }
};

void ObjectList::swap(ObjectList& other)
{
#ifndef NDEBUG
    boost::swap(sorted, other.sorted);
#endif //NDEBUG
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectList_, other.objectList_), Swap());
}

bool ObjectList::operator==(const ObjectList& other) const
{
#ifndef NDEBUG
    assert(sorted && "Unless you are being very careful with the insertion order this function requires sort to have been called.");
#endif //NDEBUG
    return objectList_ == other.objectList_;
}
bool ObjectList::operator!=(const ObjectList& other) const
{
    return !(*this == other);
}

struct Empty
{
    template<typename T>
    bool operator()(const T& toCheck)
    {
        return toCheck.empty();
    }
};

bool ObjectList::isEmpty() const
{
    return boost::fusion::all(objectList_,Empty());
}

//---------------------------------ObjectPtrList-----------------------------------
ObjectPtrList::ObjectPtrList() :
objectPtrList_()
{
}

ObjectPtrList::ObjectPtrList(const ObjectPtrList& other):
objectPtrList_(other.objectPtrList_)

{
}
ObjectPtrList& ObjectPtrList::operator=(const ObjectPtrList& other)
{
    objectPtrList_ = other.objectPtrList_;
    return *this;
}

const std::vector<const Guy*>& ObjectPtrList::getGuyListRef() const 
{
    return boost::fusion::at_c<object_list_detail::guyList>(objectPtrList_);
}
const std::vector<const Box*>& ObjectPtrList::getBoxListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxList>(objectPtrList_);
}
const std::vector<const Button*>& ObjectPtrList::getButtonListRef() const
{
    return boost::fusion::at_c<object_list_detail::buttonList>(objectPtrList_);
}
const std::vector<const Platform*>& ObjectPtrList::getPlatformListRef() const
{
    return boost::fusion::at_c<object_list_detail::platformList>(objectPtrList_);
}
const std::vector<const Portal*>& ObjectPtrList::getPortalListRef() const
{
    return boost::fusion::at_c<object_list_detail::portalList>(objectPtrList_);
}
const std::vector<const RemoteDepartureEdit<Box>* >& ObjectPtrList::getBoxThiefListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxThiefList>(objectPtrList_);
}
const std::vector<const RemoteDepartureEdit<Box>* >& ObjectPtrList::getBoxExtraListRef() const
{
    return boost::fusion::at_c<object_list_detail::boxExtraList>(objectPtrList_);
}
const std::vector<const RemoteDepartureEdit<Guy>* >& ObjectPtrList::getGuyExtraListRef() const
{
    return boost::fusion::at_c<object_list_detail::guyExtraList>(objectPtrList_);
}


template <typename T>
struct AddressOf : std::unary_function<T&, T*>
{
    T* operator()(T& t) const
    {
        return &t;
    }
};

struct InsertAddresses
{
    template<typename Container, typename SinglePassRange>
    void operator()(Container& toInsertInto, const SinglePassRange& toInsert) const
    {
        boost::push_back(toInsertInto, toInsert | boost::adaptors::transformed(AddressOf<const typename SinglePassRange::value_type>()));
    }
};

void ObjectPtrList::add(const ObjectList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectPtrList_, other.objectList_), InsertAddresses());           
}

template<typename T>
struct DereferenceLess {
    bool operator()(const T& l, const T& r) const
    {
        return *l < *r;
    }
};

//MUST CALL this before calling operator== on this ObjectPtrList
void ObjectPtrList::sort()
{
    boost::fusion::for_each(objectPtrList_, Sort<DereferenceLess>());
}

void ObjectPtrList::swap(ObjectPtrList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectPtrList_, other.objectPtrList_), Swap());
}
#if 0
template<typename T>
struct DereferenceEqual {
    bool operator()(const T& l, const T& r) const
    {
        return *l == *r;
    }
};
bool ObjectPtrList::operator==(const ObjectPtrList& other) const
{
    using boost::equal;
    return equal(guyList,other.guyList, DereferenceEqual<Guy*>())
        && equal(boxList,other.boxList, DereferenceEqual<Box*>())
        && equal(buttonList,other.buttonList, DereferenceEqual<Button*>())
        && equal(platformList,other.platformList, DereferenceEqual<Platform*>())
        && equal(portalList,other.portalList, DereferenceEqual<Portal*>())
        && equal(boxThiefList,other.boxThiefList, DereferenceEqual<RemoteDepartureEdit<Box>*>())
        && equal(guyExtraList,other.guyExtraList, DereferenceEqual<RemoteDepartureEdit<Box>*>())
        && equal(boxExtraList,other.boxExtraList, DereferenceEqual<RemoteDepartureEdit<Guy>*>());
}
bool ObjectPtrList::operator!=(const ObjectPtrList& other) const
{
    return !(*this == other);
}
#endif
} //namespace hg
