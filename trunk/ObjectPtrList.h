#ifndef HG_ARRIVAL_LIST_H
#define HG_ARRIVAL_LIST_H

#include "ObjectList_fwd.h"
#include "VectorOf.h"

#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/find.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range.hpp>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_pointer.hpp>

namespace hg {
template<typename ListTypes>
class ObjectPtrList
{
public:
    ObjectPtrList();

    template<typename ObjectT>
    boost::indirected_range<const typename vector_of<const ObjectT*>::type > getList() const;

    void add(const ObjectList<ListTypes>& other);

    //MUST CALL this to make lists sorted (required for deterministic physics)
    void sort();

    void swap(ObjectPtrList& other);

private:
    typedef typename
    boost::mpl::transform<
        ListTypes,
        vector_of<
            boost::add_pointer<
                boost::add_const<boost::mpl::_1> > > >::type ArrivalListType;
    ArrivalListType arrivalList_;
};

template<typename ListTypes>
void swap(ObjectPtrList<ListTypes>& l, ObjectPtrList<ListTypes>& r);

template<typename ListTypes>
template<typename ObjectT>
boost::indirected_range<const typename vector_of<const ObjectT*>::type > 
ObjectPtrList<ListTypes>::getList() const
{
    return boost::adaptors::indirect(
        boost::fusion::deref(
            boost::fusion::find<
                typename vector_of<const ObjectT*>::type
            >(arrivalList_)));
}
}
#endif //HG_ARRIVAL_LIST_H
