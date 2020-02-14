#ifndef HG_MP_BOOST_CONTAINER_STABLE_VECTOR
#define HG_MP_BOOST_CONTAINER_STABLE_VECTOR
#include "hg/Util/memory_pool.h"
#include <boost/container/stable_vector.hpp>
namespace hg {
namespace mp {
namespace boost{
namespace container {
template<typename T>
using stable_vector =
        ::boost::container::stable_vector<
            T,
            memory_pool_allocator<T>>;
}
}
}
}
#endif //HG_MP_BOOST_CONTAINER_STABLE_VECTOR
