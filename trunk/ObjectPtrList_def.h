#ifndef HG_ARRIVAL_LIST_DEF_H
#define HG_ARRIVAL_LIST_DEF_H
#include "ObjectPtrList.h"
#include "ObjectList.h"
#include "ObjectListHelpers.h"

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <functional>

namespace hg {
template<typename ListType>
ObjectPtrList<ListType>::ObjectPtrList() :
        arrivalList_()
{
}
template<typename ListTypes>
ObjectPtrList<ListTypes>::ObjectPtrList(const ObjectPtrList<ListTypes>& other):
        arrivalList_(other.arrivalList_)
{
}
template<typename ListTypes>
ObjectPtrList<ListTypes>& ObjectPtrList<ListTypes>::operator=(const ObjectPtrList<ListTypes>& other)
{
    arrivalList_ = other.arrivalList_;
    return *this;
}
namespace {
template<typename T>
struct DereferenceLess {
	bool operator()(const T& l, const T& r) const
	{
		return *l < *r;
	}
};
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
        boost::push_back(
            toInsertInto,
            toInsert |
                boost::adaptors::transformed(
                    AddressOf<const typename boost::range_value<SinglePassRange>::type>()));
    }
};
}//namespace
template<typename ListTypes>
void ObjectPtrList<ListTypes>::add(const ObjectList<ListTypes>& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(arrivalList_, other.departureList_), InsertAddresses());
}
//MUST CALL this before calling operator== on this ObjectPtrList<Normal> 
template<typename ListTypes>
void ObjectPtrList<ListTypes>::sort()
{
    boost::fusion::for_each(arrivalList_, Sort<DereferenceLess>());
}
template<typename ListTypes>
void ObjectPtrList<ListTypes>::swap(ObjectPtrList<ListTypes>& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(arrivalList_, other.arrivalList_), Swap());
}
template<typename ListTypes>
void swap(ObjectPtrList<ListTypes>& l, ObjectPtrList<ListTypes>& r)
{
    l.swap(r);
}
}//namespace hg
#endif //HG_ARRIVAL_LIST_DEF_H
