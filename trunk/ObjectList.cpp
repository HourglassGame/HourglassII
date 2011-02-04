#include "ObjectList.h"

#include <boost/utility/swap.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/lambda/lambda.hpp>

#include <iterator>

#include <cassert>

namespace {
template<typename T>
struct EqualIndices {
    bool operator()(const T& l, const T& r) {
        return l.getIndex() == r.getIndex();
    }
};

template<typename T>
struct EqualIndices<T*> {
    bool operator()(T* l, T* r) {
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
} //namespace

namespace hg {
ObjectList::ObjectList() :
#ifndef NDEBUG
sorted(true),
#endif //NDEBUG
guyList(),
boxList(),
buttonList(),
platformList(),
portalList(),
boxThiefList(),
boxExtraList(),
guyExtraList()
{
}

ObjectList::ObjectList(const ObjectList& other):
#ifndef NDEBUG
sorted(other.sorted),
#endif //NDEBUG
guyList(other.guyList),
boxList(other.boxList),
buttonList(other.buttonList),
platformList(other.platformList),
portalList(other.portalList),
boxThiefList(other.boxThiefList),
boxExtraList(other.boxExtraList),
guyExtraList(other.guyExtraList)
{
}
ObjectList& ObjectList::operator=(const ObjectList& other)
{
#ifndef NDEBUG
    sorted = other.sorted;
#endif //NDEBUG
    guyList = other.guyList;
    boxList = other.boxList;
    buttonList = other.buttonList;
    platformList = other.platformList;
    portalList = other.portalList;
    boxThiefList = other.boxThiefList;
    boxExtraList = other.boxExtraList;
    guyExtraList = other.guyExtraList;
    return *this;
}
void ObjectList::add(const Guy& toCopy)
{
    guyList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Box& toCopy)
{
    boxList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Button& toCopy)
{
    buttonList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Platform& toCopy)
{
    platformList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::add(const Portal& toCopy)
{
    portalList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addThief(const RemoteDepartureEdit<Box>& toCopy)
{
    boxThiefList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addExtra(const RemoteDepartureEdit<Box>& toCopy)
{
    boxExtraList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
void ObjectList::addExtra(const RemoteDepartureEdit<Guy>& toCopy)
{
    guyExtraList.push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

void ObjectList::add(const ObjectList& other)
{
    guyList.insert(guyList.end(),other.guyList.begin(),other.guyList.end());
    boxList.insert(boxList.end(),other.boxList.begin(),other.boxList.end());
    buttonList.insert(buttonList.end(),other.buttonList.begin(),other.buttonList.end());
    platformList.insert(platformList.end(),other.platformList.begin(),other.platformList.end());
    portalList.insert(portalList.end(),other.portalList.begin(),other.portalList.end());
    boxThiefList.insert(boxThiefList.end(),other.boxThiefList.begin(),other.boxThiefList.end());
    guyExtraList.insert(guyExtraList.end(),other.guyExtraList.begin(),other.guyExtraList.end());
    boxExtraList.insert(boxExtraList.end(),other.boxExtraList.begin(),other.boxExtraList.end());
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

//MUST CALL THIS before calling operator== constructing complete ObjectList!
void ObjectList::sort()
{
    using boost::sort;
    //All the lists with the assert(containsNoElementsWithEqualIndices) are sorted on index alone, but have equality
    //based on all their members. This means that they must never have more than one element with the same index, 
    //because that could cause equal ObjectLists to be found to be different because the order in which the lists within the
    //two ObjectLists are sorted is different, even though the lists contain the same elements.
	sort(guyList);
    assert(containsNoElementsWithEqualIndices(guyList));
	sort(boxList);
	sort(buttonList);
    assert(containsNoElementsWithEqualIndices(buttonList));
	sort(platformList);
    assert(containsNoElementsWithEqualIndices(platformList));
	sort(portalList);
    assert(containsNoElementsWithEqualIndices(portalList));
	sort(boxThiefList);
	sort(guyExtraList);
	sort(boxExtraList);
#ifndef NDEBUG
    sorted = true;
#endif //NDEBUG
}

void ObjectList::swap(ObjectList& other)
{
#ifndef NDEBUG
    boost::swap(sorted, other.sorted);
#endif //NDEBUG
    guyList.swap(other.guyList);
    boxList.swap(other.boxList);
    buttonList.swap(other.buttonList);
    platformList.swap(other.platformList);
    portalList.swap(other.portalList);
    boxThiefList.swap(other.boxThiefList);
    guyExtraList.swap(other.guyExtraList);
    boxExtraList.swap(other.boxExtraList);
}

bool ObjectList::operator==(const ObjectList& other) const
{
#ifndef NDEBUG
    assert(sorted && "Unless you are being very careful with the insertion order this function requires sort to have been called.");
#endif //NDEBUG
    using boost::equal;
    return equal(guyList,other.guyList)
        && equal(boxList,other.boxList)
        && equal(buttonList,other.buttonList)
        && equal(platformList,other.platformList)
        && equal(portalList,other.portalList)
        && equal(boxThiefList,other.boxThiefList)
        && equal(guyExtraList,other.guyExtraList)
        && equal(boxExtraList,other.boxExtraList);
}
bool ObjectList::operator!=(const ObjectList& other) const
{
    return !(*this == other);
}
bool ObjectList::isEmpty() const
{
    return guyList.empty()
        && boxList.empty()
        && buttonList.empty()
        && platformList.empty()
        && portalList.empty()
        && boxThiefList.empty()
        && guyExtraList.empty()
        && boxExtraList.empty();
}



//---------------------------------ObjectPtrList-----------------------------------
ObjectPtrList::ObjectPtrList() :
guyList(),
boxList(),
buttonList(),
platformList(),
portalList(),
boxThiefList(),
boxExtraList(),
guyExtraList()
{
}

ObjectPtrList::ObjectPtrList(const ObjectPtrList& other):
guyList(other.guyList),
boxList(other.boxList),
buttonList(other.buttonList),
platformList(other.platformList),
portalList(other.portalList),
boxThiefList(other.boxThiefList),
boxExtraList(other.boxExtraList),
guyExtraList(other.guyExtraList)
{
}
ObjectPtrList& ObjectPtrList::operator=(const ObjectPtrList& other)
{
    guyList = other.guyList;
    boxList = other.boxList;
    buttonList = other.buttonList;
    platformList = other.platformList;
    portalList = other.portalList;
    boxThiefList = other.boxThiefList;
    boxExtraList = other.boxExtraList;
    guyExtraList = other.guyExtraList;
    return *this;
}

template <typename T>
struct AddressOf : std::unary_function<T&, T*>
{
    T* operator()(T& t) const
    {
        return &t;
    }
};

void ObjectPtrList::add(const ObjectList& other)
{
    using boost::adaptors::transformed;
    using boost::push_back;
    push_back(guyList, other.guyList | transformed(AddressOf<const Guy>()));
    push_back(boxList, other.boxList | transformed(AddressOf<const Box>()));
    push_back(buttonList, other.buttonList | transformed(AddressOf<const Button>()));
    push_back(platformList, other.platformList | transformed(AddressOf<const Platform>()));
    push_back(portalList, other.portalList | transformed(AddressOf<const Portal>()));
    push_back(boxThiefList, other.boxThiefList | transformed(AddressOf<const RemoteDepartureEdit<Box> >()));
    push_back(boxExtraList, other.boxExtraList | transformed(AddressOf<const RemoteDepartureEdit<Box> >()));
    push_back(guyExtraList, other.guyExtraList | transformed(AddressOf<const RemoteDepartureEdit<Guy> >()));            
}

//MUST CALL this before calling operator== on this ObjectPtrList
void ObjectPtrList::sort()
{
    using boost::sort;
    using namespace boost::lambda;
	sort(guyList, *_1 < *_2);
    assert(containsNoElementsWithEqualIndices(guyList));
	sort(boxList, *_1 < *_2);
	sort(buttonList, *_1 < *_2);
    assert(containsNoElementsWithEqualIndices(buttonList));
	sort(platformList, *_1 < *_2);
    assert(containsNoElementsWithEqualIndices(platformList));
	sort(portalList, *_1 < *_2);
    assert(containsNoElementsWithEqualIndices(portalList));
	sort(boxThiefList, *_1 < *_2);
	sort(boxExtraList, *_1 < *_2);
	sort(guyExtraList, *_1 < *_2);
}

void ObjectPtrList::swap(ObjectPtrList& other)
{
    guyList.swap(other.guyList);
    boxList.swap(other.boxList);
    buttonList.swap(other.buttonList);
    platformList.swap(other.platformList);
    portalList.swap(other.portalList);
    boxThiefList.swap(other.boxThiefList);
    guyExtraList.swap(other.guyExtraList);
    boxExtraList.swap(other.boxExtraList);
}
#if 0
template<typename T>
struct DereferenceEqual {
    bool operator()(const T& l, const T& r) const
    {
        return *l == *r;
    }
}
bool ObjectPtrList::operator==(const ObjectPtrList& other) const
{
    using boost::equal;
    return equal(guyList,other.guyList, DereferenceEqual<Guy*>())
        && equal(boxList,other.boxList,DereferenceEqual<Box*>())
        && equal(buttonList,other.buttonList,DereferenceEqual<Button*>())
        && equal(platformList,other.platformList,DereferenceEqual<Platform*>())
        && equal(portalList,other.portalList,DereferenceEqual<Portal*>())
        && equal(boxThiefList,other.boxThiefList,DereferenceEqual<RemoteDepartureEdit<Box>*>())
        && equal(guyExtraList,other.guyExtraList,DereferenceEqual<RemoteDepartureEdit<Box>*>())
        && equal(boxExtraList,other.boxExtraList,DereferenceEqual<RemoteDepartureEdit<Guy>*>());
}
bool ObjectPtrList::operator!=(const ObjectPtrList& other) const
{
    return !(*this == other);
}
#endif
} //namespace hg