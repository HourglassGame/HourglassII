#ifndef HG_MT_BOOST_UNORDERED_MAP_HPP
#define HG_MT_BOOST_UNORDERED_MAP_HPP
#include "hg/Util/multi_thread_allocator.h"
#include <boost/unordered_map.hpp>
namespace hg {
namespace mt {
namespace boost{
template<
    typename Key,
    typename Mapped,
    typename Hash = ::boost::hash<Key>,
    typename Pred = ::std::equal_to<Key>
>
using unordered_map =
        ::boost::unordered_map<
            Key,
            Mapped,
            Hash,
            Pred,
            multi_thread_allocator_t< ::std::pair<Key const, Mapped>>>;
}
}
}
#endif //HG_MT_BOOST_UNORDERED_MAP_HPP
