#ifndef HG_DEPARTURE_LIST_DEF_H
#define HG_DEPARTURE_LIST_DEF_H
#include "ObjectList.h"

#include "ObjectListHelpers.h"
#include "SortWeakerThanEquality.h"

#include <boost/range/algorithm_ext/push_back.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <cassert>

namespace hg {
template<typename ListTypes>
ObjectList<ListTypes>::ObjectList() :
#ifndef NDEBUG
        sorted(true),
#endif //NDEBUG
        list_()
{
}
template<typename ListTypes>
ObjectList<ListTypes>::ObjectList(ObjectList const& other) :
#ifndef NDEBUG
    sorted(other.sorted),
#endif //NDEBUG
    list_(other.list_)
{
}
template<typename ListTypes>
ObjectList<ListTypes>& ObjectList<ListTypes>::operator=(BOOST_COPY_ASSIGN_REF(ObjectList) other)
{
#ifndef NDEBUG
    sorted = other.sorted;
#endif //NDEBUG
    list_ = other.list_;
    return *this;
}

template<typename ListTypes>
ObjectList<ListTypes>::ObjectList(BOOST_RV_REF(ObjectList) other) :
#ifndef NDEBUG
    sorted(boost::move(other.sorted)),
#endif //NDEBUG
    list_(boost::move(other.list_))
{
}
template<typename ListTypes>
ObjectList<ListTypes>& ObjectList<ListTypes>::operator=(BOOST_RV_REF(ObjectList) other)
{
#ifndef NDEBUG
    sorted = boost::move(other.sorted);
#endif //NDEBUG
    list_ = boost::move(other.list_);
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
template<typename ListTypes>
void ObjectList<ListTypes>::add(const ObjectList<ListTypes>& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(list_, other.list_), Insert());
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

//MUST CALL THIS before calling operator== constructing complete ObjectList<Normal> !
template<typename ListTypes>
void ObjectList<ListTypes>::sort()
{
    boost::fusion::for_each(list_, Sort());
#ifndef NDEBUG
    sorted = true;
#endif //NDEBUG
}
template<typename ListTypes>
void ObjectList<ListTypes>::swap(ObjectList<ListTypes>& other)
{
#ifndef NDEBUG
    boost::swap(sorted, other.sorted);
#endif //NDEBUG
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(list_, other.list_), Swap());
}
template<typename ListTypes>
bool ObjectList<ListTypes>::operator==(const ObjectList<ListTypes>& other) const
{
#ifndef NDEBUG
    assert(
        sorted
        && other.sorted 
        && "Unless you are being very careful with the insertion order this function requires sort to have been called.");
#endif //NDEBUG
    return list_ == other.list_;
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
template<typename ListTypes>
bool ObjectList<ListTypes>::isEmpty() const
{
    return boost::fusion::all(list_, Empty());
}
template<typename ObjectT>
void swap(ObjectList<ObjectT>& l, ObjectList<ObjectT>& r)
{
    l.swap(r);
}
} //namespace hg
#endif //HG_DEPARTURE_LIST_DEF_H