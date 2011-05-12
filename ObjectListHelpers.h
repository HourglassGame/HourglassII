#ifndef HG_OBJECTLISTHELPERS_H
#define HG_OBJECTLISTHELPERS_H
#include "SortWeakerThanEquality.h"
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/zip_view.hpp>
#include <boost/fusion/functional.hpp>

#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/utility/swap.hpp>

#include <cassert>

namespace {
template<class SequenceOfSequences, class Func>
void n_ary_for_each(SequenceOfSequences const & s, Func const & f)
{
    using namespace boost::fusion;
    for_each(zip_view<SequenceOfSequences>(s),
             fused_procedure<Func const &>(f));
}
struct Swap
{
    template<typename T1, typename T2>
    void operator()(T1& lhs, T2& rhs) const
    {
        boost::swap(lhs, rhs);
    }
};

template<typename T>
struct Equivalent
{
    bool operator()(const T& l, const T& r) const {
        return !(l < r) && !(r < l);
    }
};

template<typename T>
struct Equivalent<T*>
{
    bool operator()(T* l, T* r) const {
        return !(*l < *r) && !(*r < *l);
    }
};

template<typename ForwardRange>
bool containsNoEquivalentElements(const ForwardRange& range)
{
    return boost::adjacent_find<boost::return_found>(
            range,
            Equivalent<
                typename boost::range_value<ForwardRange>::type
            >())
           == boost::end(range);
}

template<template<class> class Comparitor = std::less>
struct Sort
{
    template <typename ListType>
    void operator()(ListType& toSort, typename boost::disable_if<hg::sort_weaker_than_equality<ListType> >::type* = 0) const
    {
        boost::sort(toSort, Comparitor<typename ListType::value_type>());
    }
    template <typename ListType>
    void operator()(ListType& toSort, typename boost::enable_if<hg::sort_weaker_than_equality<ListType> >::type* = 0) const
    {
        boost::sort(toSort, Comparitor<typename ListType::value_type>());
        //These lists are sorted on a criterion that allows equivalent elements to not be equal.
        //This means that they must never have equivalent elements, because that could cause equal
        //ObjectLists to be found to be different because the order in which the lists within the
        //two ObjectLists are sorted is different, even though the lists contain the same elements.
        assert(containsNoEquivalentElements(toSort));
    }
};
}//namespace
#endif //HG_OBJECTLISTHELPERS_H

