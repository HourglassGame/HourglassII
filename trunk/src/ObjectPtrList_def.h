#ifndef HG_ARRIVAL_LIST_DEF_H
#define HG_ARRIVAL_LIST_DEF_H
#include "ObjectPtrList.h"
#include "ObjectList.h"
#include "ObjectListHelpers.h"

#include "Foreach.h"

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
struct InsertAddresses
{
    template<typename Container, typename ForwardRange>
    void operator()(Container& toInsertInto, ForwardRange const& toInsert) const
    {
        toInsertInto.reserve(boost::distance(toInsertInto) + boost::distance(toInsert));
        typedef typename boost::range_value<ForwardRange>::type ObjType;
        foreach (ObjType const& obj, toInsert) {
            toInsertInto.push_back(
            		typename ConstPtr_of<ObjType>::type(obj));
        }
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
