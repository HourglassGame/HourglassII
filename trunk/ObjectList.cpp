#include "ObjectList.h"

#include "ObjectListHelpers.h"
#include "SortWeakerThanEquality.h"

#include <boost/range/algorithm_ext/push_back.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/vector_tie.hpp>

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
