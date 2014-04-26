#ifndef HG_NATURAL_SORT_H
#define HG_NATURAL_SORT_H
#include <functional>
#include <iterator>
#include <boost/range.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>
#include  <cctype>
namespace hg {
template<typename T>
bool chunkwise_less(T const &l, T const &r) {
    if (boost::empty(l) && boost::empty(r)) {
        assert(false && "This shouldn't happen for current usages of chunkwise_less; if this assert triggers, check that you are being sensible, and perhaps remove this assert.");
        return false;
    }
    else if (boost::empty(l)) {
        return true;
    }
    else if (boost::empty(r)) {
        return false;
    }
    else if (std::isdigit(*boost::begin(l)) && std::isdigit(*boost::begin(r))) {
        if (boost::distance(l) == boost::distance(r)) {
            return boost::range::lexicographical_compare(l, r);
        }
        return boost::distance(l) < boost::distance(r);
    }
    else {
        return boost::algorithm::ilexicographical_compare(l, r);
    }
}

template<typename Iterator>
Iterator advance_to_end_of_chunk(Iterator begin, Iterator const& end) {
    if (begin != end) {
        bool numeric = std::isdigit(*begin);
        do ++begin; while (begin != end && std::isdigit(*begin) == numeric);
    }
    return begin;
}

template<typename T1, typename T2>
bool natural_less(T1 const &l, T2 const &r) {
    auto lbegin = std::begin(l);
    auto rbegin = std::begin(r);
    auto const lend = std::end(l);
    auto const rend = std::end(r);
    
    while (lbegin != lend && rbegin != rend) {
        auto lchunkend = advance_to_end_of_chunk(lbegin, lend);
        auto rchunkend = advance_to_end_of_chunk(rbegin, rend);
    
        if (chunkwise_less(
                boost::make_iterator_range(lbegin, lchunkend),
                boost::make_iterator_range(rbegin, rchunkend)))
        {
            //Left chunk < Right chunk, so left < right
            return true;
        }
        if (!boost::iequals(
                boost::make_iterator_range(lbegin, lchunkend),
                boost::make_iterator_range(rbegin, rchunkend)))
        {
            //!(Left chunk < Right chunk) and leftchunk != rightchunk, so left > right
            return false;
        }
        lbegin = lchunkend;
        rbegin = rchunkend;
    }
    //all chunks equivalent, so left == right
    return false;
}

template<typename T>
struct natural_less_struct : std::binary_function <T,T,bool>  {
    bool operator()(T const &l, T const &r) const {
        return natural_less(l,r);
    }
};
}
#endif //HG_NATURAL_SORT_H
