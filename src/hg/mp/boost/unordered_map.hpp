#ifndef HG_MP_BOOST_UNORDERED_MAP_HPP
#define HG_MP_BOOST_UNORDERED_MAP_HPP
#include "hg/Util/memory_pool.h"
#include <boost/unordered_map.hpp>
namespace hg {
namespace mp {
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
            memory_pool_allocator< ::std::pair<Key const, Mapped>>>;
}
}
}
#endif //HG_MP_BOOST_UNORDERED_MAP_HPP
