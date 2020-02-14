#ifndef HG_MP_BOOST_CONTAINER_MAP
#define HG_MP_BOOST_CONTAINER_MAP
#include "hg/Util/memory_pool.h"
#include <boost/container/map.hpp>
namespace hg {
namespace mp {
namespace boost {
namespace container {
template<
    typename Key,
    typename T,
    typename Compare = ::std::less<Key>>
using map =
    ::boost::container::map<
        Key,
        T,
        Compare,
        memory_pool_allocator< ::std::pair<Key const, T>>>;
}
}
}
}
#endif //HG_MP_BOOST_CONTAINER_MAP
