#ifndef HG_MP_BOOST_CONTAINER_VECTOR
#define HG_MP_BOOST_CONTAINER_VECTOR
#include "../../../memory_pool.h"
#include <boost/container/vector.hpp>
namespace hg {
namespace mp {
namespace boost {
namespace container {
template<typename T>
using vector =
        ::boost::container::vector<
            T,
            memory_pool_allocator<T>>;
}
}
}
}
#endif //HG_MP_BOOST_CONTAINER_VECTOR
