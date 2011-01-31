#include "ObjectList.h"

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <boost/utility/addressof.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <iterator>
using namespace ::std;
using namespace ::boost;
namespace hg {
ObjectList::ObjectList() :
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

ObjectList::~ObjectList()
{
}
ObjectList::ObjectList(const ObjectList& other):
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
}
void ObjectList::add(const Box& toCopy)
{
    boxList.push_back(toCopy);
}
void ObjectList::add(const Button& toCopy)
{
    buttonList.push_back(toCopy);
}
void ObjectList::add(const Platform& toCopy)
{
    platformList.push_back(toCopy);
}
void ObjectList::add(const Portal& toCopy)
{
    portalList.push_back(toCopy);
}
void ObjectList::addThief(const RemoteDepartureEdit<Box>& toCopy)
{
    boxThiefList.push_back(toCopy);
}
void ObjectList::addExtra(const RemoteDepartureEdit<Box>& toCopy)
{
    boxExtraList.push_back(toCopy);
}
void ObjectList::addExtra(const RemoteDepartureEdit<Guy>& toCopy)
{
    guyExtraList.push_back(toCopy);
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
}
//MUST CALL THIS after constructing complete ObjectList!
void ObjectList::sort()
{
    using boost::sort;
	sort(guyList);
    //assert(containsNoGuysWithEqualRelativeIndices(guyList) && "If the list contains guys with equal relative index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(boxList);
	sort(buttonList);
    //assert(containsNoElementsWithEqualIndices(buttonList) && "If the list contains buttons with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(platformList);
    //assert(containsNoElementsWithEqualIndices(platformList) && "If the list contains platforms with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(portalList);
    //assert(containsNoElementsWithEqualIndices(portalList) && "If the list contains portals with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(boxThiefList);
	sort(guyExtraList);
	sort(boxExtraList);
}

void ObjectList::swap(ObjectList& other)
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

bool ObjectList::operator==(const ObjectList& other) const
{
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

ObjectPtrList::~ObjectPtrList()
{}
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
struct AddressOf
{
    T* operator()(const T& toTakeAddressOf) const
    {
        return const_cast<T*>(boost::addressof(toTakeAddressOf));
    }
};

void ObjectPtrList::add(const ObjectList& other)
{
    {
        std::back_insert_iterator<std::vector<const Guy*> > it(guyList);
        boost::transform(other.guyList, it, AddressOf<Guy>());
    }
    {
        std::back_insert_iterator<std::vector<const Box*> > it(boxList);
        boost::transform(other.boxList, it, AddressOf<Box>());
    }
    {
        std::back_insert_iterator<std::vector<const Button*> > it(buttonList);
        boost::transform(other.buttonList, it, AddressOf<Button>());
    }
    {
        std::back_insert_iterator<std::vector<const Platform*> > it(platformList);
        boost::transform(other.platformList, it, AddressOf<Platform>());
    }
    {
        std::back_insert_iterator<std::vector<const Portal*> > it(portalList);
        boost::transform(other.portalList, it, AddressOf<Portal>());
    }
    {
        std::back_insert_iterator<std::vector<const RemoteDepartureEdit<Box>*> > it(boxThiefList);
        boost::transform(other.boxThiefList, it, AddressOf<RemoteDepartureEdit<Box> >());
    }
    {
        std::back_insert_iterator<std::vector<const RemoteDepartureEdit<Box>*> > it(boxExtraList);
        boost::transform(other.boxExtraList, it, AddressOf<RemoteDepartureEdit<Box> >());
    }
    {
        std::back_insert_iterator<std::vector<const RemoteDepartureEdit<Guy>*> > it(guyExtraList);
        boost::transform(other.guyExtraList, it, AddressOf<RemoteDepartureEdit<Guy> >());
    }
}
template <typename T>
struct DereferenceLessThan {
    bool operator()(const T& l, const T& r) const
    {
        return *l < *r;
    }
};

//MUST CALL THIS after constructing complete ObjectPtrList!
void ObjectPtrList::sort()
{
    using boost::sort;
	sort(guyList,DereferenceLessThan<const Guy*>());
    //assert(containsNoGuyPointersWithEqualRelativeIndices(guyList) && "If the list contains guys with equal relative index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(boxList,DereferenceLessThan<const Box*>());
	sort(buttonList,DereferenceLessThan<const Button*>());
    
    //assert(containsNoElementPointersWithEqualIndices(buttonList) && "If the list contains buttons with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(platformList,DereferenceLessThan<const Platform*>());
    //assert(containsNoElementPointersWithEqualIndices(platformList) && "If the list contains platforms with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(portalList,DereferenceLessThan<const Portal*>());
    //assert(containsNoElementPointersWithEqualIndices(portalList) && "If the list contains portals with equal index then "
    //                                                                 "the sort order is non-deterministic, potentially leading "
    //                                                                 "equal objectLists being found to be different");
	sort(boxThiefList,DereferenceLessThan<const RemoteDepartureEdit<Box>*>());
	sort(boxExtraList,DereferenceLessThan<const RemoteDepartureEdit<Box>*>());
	sort(guyExtraList,DereferenceLessThan<const RemoteDepartureEdit<Guy>*>());
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