#ifndef HG_NATURAL_SORT_H
#define HG_NATURAL_SORT_H
#include <functional>
#include <iterator>
#include <boost/range.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>
#include <cctype>
namespace hg {
template<typename T>
bool chunkwise_less(T const &l, T const &r) {
    using value_type = typename boost::range_value<T>::type;
    static_assert(
        std::is_same<value_type, char>::value
     || std::is_same<value_type, unsigned char>::value
     || std::is_same<value_type, signed char>::value,
        "std::isdigit requires values to be within the range of an unsigned char");
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
    //TODO: Correctly handle non-ascii encoding with isdigit
    else if (std::isdigit(static_cast<unsigned char>(*boost::begin(l))) && std::isdigit(static_cast<unsigned char>(*boost::begin(r)))) {
        if (boost::size(l) == boost::size(r)) {
            return boost::range::lexicographical_compare(l, r);
        }
        return boost::size(l) < boost::size(r);
    }
    else {
        return boost::algorithm::ilexicographical_compare(l, r);
    }
}

template<typename Iterator>
Iterator advance_to_end_of_chunk(Iterator begin, Iterator const &end) {
    using value_type = typename boost::iterator_value<Iterator>::type;
    static_assert(
           std::is_same<value_type, char>::value
        || std::is_same<value_type, unsigned char>::value
        || std::is_same<value_type, signed char>::value,
        "std::isdigit requires values to be within the range of an unsigned char");

    if (begin != end) {
        bool const numeric = std::isdigit(static_cast<unsigned char>(*begin));
        do { ++begin; } while (begin != end && !!std::isdigit(static_cast<unsigned char>(*begin)) == numeric);
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

template<typename T = void>
struct natural_less_struct final {
    bool operator()(T const &l, T const &r) const {
        return natural_less(l,r);
    }
};
template<>
struct natural_less_struct<void> final {
    template<typename T>
    bool operator()(T const &l, T const &r) const {
        return natural_less(l, r);
    }
};
}
#endif //HG_NATURAL_SORT_H
