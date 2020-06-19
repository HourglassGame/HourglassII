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
		list()
{
}

namespace {
struct InsertAddresses final
{
	template<typename Container, typename ForwardRange>
	void operator()(Container &toInsertInto, ForwardRange const &toInsert) const
	{
		toInsertInto.reserve(boost::size(toInsertInto) + boost::size(toInsert));
		typedef typename boost::range_value<ForwardRange>::type ObjType;
		for (ObjType const &obj: toInsert) {
			toInsertInto.push_back(
					typename ConstPtr_of<ObjType>::type(obj));
		}
	}
};
}//namespace
template<typename ListTypes>
void ObjectPtrList<ListTypes>::add(ObjectList<ListTypes> const &o)
{
	using namespace boost::fusion;
	n_ary_for_each(vector_tie(list, o.list), InsertAddresses());
}
//MUST CALL this before calling operator== on this ObjectPtrList<Normal> 
template<typename ListTypes>
void ObjectPtrList<ListTypes>::sort()
{
	boost::fusion::for_each(list, Sort());
}
template<typename ListTypes>
void ObjectPtrList<ListTypes>::swap(ObjectPtrList<ListTypes> &o)
{
	using namespace boost::fusion;
	n_ary_for_each(vector_tie(list, o.list), Swap());
}
template<typename ListTypes>
void swap(ObjectPtrList<ListTypes> &l, ObjectPtrList<ListTypes> &r)
{
	l.swap(r);
}
}//namespace hg

#endif //HG_ARRIVAL_LIST_DEF_H
