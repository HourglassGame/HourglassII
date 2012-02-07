#ifndef HG_DEPARTURE_LIST_H
#define HG_DEPARTURE_LIST_H
#include "ObjectPtrList_fwd.h"
#include "VectorOf.h"
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#include <boost/fusion/mpl.hpp>
#include <boost/fusion/include/find.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range.hpp>

#include <boost/move/move.hpp>

#include <boost/operators.hpp>

namespace hg {
// Object list stores all data sent between frames or to rendering engine
template<typename ListTypes>
class ObjectList : boost::equality_comparable<ObjectList<ListTypes> >
{
public:
    ObjectList();
    ObjectList(ObjectList const& o);
    ObjectList& operator=(BOOST_COPY_ASSIGN_REF(ObjectList) o);
    ObjectList(BOOST_RV_REF(ObjectList) o);
    ObjectList& operator=(BOOST_RV_REF(ObjectList) o);
    
    template<typename ObjectT>
    typename vector_of<ObjectT>::type const& getList() const;
    
    template<typename ObjectT>
    void add(ObjectT const& toCopy);
    
    template<typename ObjectRangeT>
    void addRange(ObjectRangeT const& toAdd);

    void add(ObjectList const& o);
    //MUST CALL this to make lists sorted (required for operator==)
    void sort();

    void swap(ObjectList& o);

    bool operator==(ObjectList const& o) const;
    bool isEmpty() const;
private:
    friend class ObjectPtrList<ListTypes>;
#ifndef NDEBUG
    bool sorted;
#endif //NDEBUG
    typedef typename
    boost::mpl::transform<
        ListTypes,
        vector_of<boost::mpl::_1> >::type ListType;
    ListType list_;
    BOOST_COPYABLE_AND_MOVABLE(ObjectList)
};
template<typename ObjectT>
void swap(ObjectList<ObjectT>& l, ObjectList<ObjectT>& r);
template<typename ListTypes>
template<typename ObjectT>
void ObjectList<ListTypes>::add(ObjectT const& toCopy)
{
    boost::fusion::deref(
        boost::fusion::find<
            typename vector_of<ObjectT>::type
        >(list_)
    ).push_back(toCopy);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}
template<typename ListTypes>
template<typename ObjectRangeT>
void ObjectList<ListTypes>::addRange(const ObjectRangeT& toAdd)
{
    boost::push_back(
        boost::fusion::deref(
            boost::fusion::find<
                typename vector_of<typename boost::range_value<ObjectRangeT>::type >::type
            >(list_)),
        toAdd);
#ifndef NDEBUG
    sorted = false;
#endif //NDEBUG
}

//Be very careful about using this on unsorted lists.
//REALLY! BE CAREFUL. You could end up with non-deterministic/unstable stuff 
//if you have code which depends on the order of the resulting range.
template<typename ListTypes>
template<typename ObjectT>
typename vector_of<ObjectT>::type const& ObjectList<ListTypes>::getList() const
{
    return boost::fusion::deref(
        boost::fusion::find<
            typename vector_of<ObjectT>::type
        >(list_));
}
}
#endif //HG_DEPARTURE_LIST_H
