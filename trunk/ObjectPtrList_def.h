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
        list_()
{
}

namespace {

template <typename T>
struct ConstAddressOf : std::unary_function<T const&, typename ConstPtr_of<T>::type>
{
    typename ConstPtr_of<T>::type operator()(T const& t) const
    {
        return typename ConstPtr_of<T>::type(t);
    }
};
struct InsertAddresses
{
    template<typename Container, typename SinglePassRange>
    void operator()(Container& toInsertInto, SinglePassRange const& toInsert) const
    {
        boost::push_back(
            toInsertInto,
            toInsert |
                boost::adaptors::transformed(
                    ConstAddressOf<typename boost::range_value<SinglePassRange>::type>()));
    }
};
}//namespace
template<typename ListTypes>
void ObjectPtrList<ListTypes>::add(ObjectList<ListTypes> const& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(list_, other.list_), InsertAddresses());
}
//MUST CALL this before calling operator== on this ObjectPtrList<Normal> 
template<typename ListTypes>
void ObjectPtrList<ListTypes>::sort()
{
    boost::fusion::for_each(list_, Sort());
}
template<typename ListTypes>
void ObjectPtrList<ListTypes>::swap(ObjectPtrList<ListTypes>& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(list_, other.list_), Swap());
}
template<typename ListTypes>
void swap(ObjectPtrList<ListTypes>& l, ObjectPtrList<ListTypes>& r)
{
    l.swap(r);
}
}//namespace hg

#endif //HG_ARRIVAL_LIST_DEF_H
