#ifndef HG_ARRIVAL_LIST_H
#define HG_ARRIVAL_LIST_H

#include "ObjectList_fwd.h"
#include "VectorOf.h"
#include "ConstPtr_of.h"
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/find.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range.hpp>

namespace hg {
template <typename ObjectConstPtr>
struct GetBase : std::unary_function<ObjectConstPtr, typename ObjectConstPtr::base_type>
{
    typename ObjectConstPtr::base_type
        operator()(ObjectConstPtr ptr) const
    {
        return ptr.get();
    }
};
//Holds pointers to objects.
//These objects are not owned by ObjectPtrList, so care is needed to avoid getting invalid pointers.
//As a general rule, none of the member functions of ObjectPtrList have well defined behaviour
//if objects that had been added to the ObjectPtrList are not live.
template<typename ListTypes>
class ObjectPtrList
{
public:
    ObjectPtrList();

    template<typename ObjectT>
    boost::transformed_range<
        GetBase<
            typename ConstPtr_of<ObjectT>::type>,
        const typename vector_of<typename ConstPtr_of<ObjectT>::type >::type> getList() const;

    void add(const ObjectList<ListTypes>& other);

    template<typename SinglePassRange>
    void addRange(SinglePassRange const& toAdd);

    //MUST CALL this to make lists sorted (required for deterministic physics)
    void sort();

    void swap(ObjectPtrList& other);

private:
    typedef typename
    boost::mpl::transform<
        ListTypes,
        vector_of<
            ConstPtr_of<boost::mpl::_1> > >::type ListType;
    ListType list_;
};

template<typename ListTypes>
void swap(ObjectPtrList<ListTypes>& l, ObjectPtrList<ListTypes>& r);

template<typename ListTypes>
template<typename ObjectT>
boost::transformed_range<GetBase<typename ConstPtr_of<ObjectT>::type>,const typename vector_of<typename ConstPtr_of<ObjectT>::type >::type>
ObjectPtrList<ListTypes>::getList() const
{
    return boost::adaptors::transform(
        boost::fusion::deref(
                boost::fusion::find<
                    typename vector_of<typename ConstPtr_of<ObjectT>::type >::type
                >(list_)), 
        GetBase<typename ConstPtr_of<ObjectT>::type>());
}
template<typename ListTypes>
template<typename SinglePassRange>
void ObjectPtrList<ListTypes>::addRange(SinglePassRange const& toAdd)
{
    boost::push_back(
        boost::fusion::deref(
            boost::fusion::find<
                typename vector_of<typename ConstPtr_of<typename boost::range_value<SinglePassRange>::type>::type>::type
            >(list_)),
        toAdd);
}

}
#endif //HG_ARRIVAL_LIST_H
