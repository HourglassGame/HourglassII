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

#include <vector>

namespace hg {
// Object list stores all data sent between frames or to rendering engine
template<typename ListTypes>
class ObjectList
{
public:
    ObjectList();

    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);
    
    template<typename ObjectT>
    typename vector_of<ObjectT>::type const& getList() const;
    
    template<typename ObjectT>
    void add(const ObjectT& toCopy);
    
    template<typename ObjectRangeT>
    void addRange(const ObjectRangeT& toAdd);

    void add(const ObjectList& other);
    //MUST CALL this to make lists sorted (required for operator==)
    void sort();

    void swap(ObjectList& other);

    bool operator==(const ObjectList& other) const;
    bool operator!=(const ObjectList& other) const;
    bool isEmpty() const;
private:
    friend class ObjectPtrList<ListTypes>;
#ifndef NDEBUG
    bool sorted;
#endif //NDEBUG
    typedef typename
    boost::mpl::transform<
        ListTypes,
        vector_of<boost::mpl::_1> >::type DepartureListType;
    DepartureListType departureList_;
};
template<typename ObjectT>
void swap(ObjectList<ObjectT>& l, ObjectList<ObjectT>& r);
template<typename ListTypes>
template<typename ObjectT>
void ObjectList<ListTypes>::add(const ObjectT& toCopy)
{
    boost::fusion::deref(
        boost::fusion::find<
            typename vector_of<ObjectT>::type
        >(departureList_)
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
            >(departureList_)),
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
        >(departureList_));
}
}
#endif //HG_DEPARTURE_LIST_H
