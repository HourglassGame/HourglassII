#include "ObjectList.h"

#include "ObjectListHelpers.h"
#include "SortWeakerThanEquality.h"

#include <boost/range/algorithm_ext/push_back.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <cassert>

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
const std::vector<TriggerData >& ObjectList::getTriggerDataListRef() const
{
    return boost::fusion::at_c<object_list_detail::triggerDataList>(objectList_);
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
void ObjectList::add(const TriggerData& toCopy)
{
    boost::fusion::at_c<object_list_detail::triggerDataList>(objectList_).push_back(toCopy);
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

namespace {
	struct Insert
	{
		template<typename Container, typename SinglePassRange>
		void operator()(Container& toInsertInto, const SinglePassRange& toInsert) const
		{
			boost::push_back(toInsertInto, toInsert);
		}
	};
}

void ObjectList::add(const ObjectList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectList_, other.objectList_), Insert());
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

//MUST CALL THIS before calling operator== constructing complete ObjectList!
void ObjectList::sort()
{
    boost::fusion::for_each(objectList_, Sort<>());
#ifndef NDEBUG
    sorted = true;
#endif //NDEBUG
}

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

namespace {
	struct Empty
	{
		template<typename T>
		bool operator()(const T& toCheck)
		{
			return toCheck.empty();
		}
	};
}

bool ObjectList::isEmpty() const
{
    return boost::fusion::all(objectList_, Empty());
}
} //namespace hg
