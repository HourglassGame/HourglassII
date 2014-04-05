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
bool compare_chunks(T const &l, T const &r) {
    if (boost::empty(l) && boost::empty(r)) {
        //assert(false);
        return true;
    }
    else if (boost::empty(l)) {
        return true;
    }
    else if (boost::empty(r)) {
        return false;
    }
    else if (std::isdigit(*boost::begin(l)) && std::isdigit(*boost::begin(r))) {
        if ( boost::distance(l) == boost::distance(r)) {
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
    bool numeric;
    if (begin != end) numeric = std::isdigit(*begin);
    do ++begin; while (begin != end && std::isdigit(*begin) == numeric);
    return begin;
}

template<typename T>
bool natural_less(T const &l, T const &r) {
    auto lbegin = std::begin(l);
    auto rbegin = std::begin(r);
    auto const lend = std::end(l);
    auto const rend = std::end(r);
    
    while (true) {
        auto lchunkend = advance_to_end_of_chunk(lbegin, lend);
        auto rchunkend = advance_to_end_of_chunk(rbegin, rend);
    
        if (compare_chunks(boost::make_iterator_range(lbegin, lchunkend), boost::make_iterator_range(rbegin, rchunkend))) return true;
        if (!boost::equal(boost::make_iterator_range(lbegin, lchunkend), boost::make_iterator_range(rbegin, rchunkend))) return false;
        lbegin = lchunkend;
        rbegin = rchunkend;
    }
}

template<typename T>
struct natural_less_struct : std::binary_function <T,T,bool>  {
    bool operator()(T const &l, T const &r) const {
        return natural_less(l,r);
    }
};
}
#endif //HG_NATURAL_SORT_H
