#ifndef HG_MT_BOOST_CONTAINER_VECTOR
#define HG_MT_BOOST_CONTAINER_VECTOR
#include "../../../multi_thread_allocator.h"
#include <boost/container/vector.hpp>
namespace hg {
namespace mt {
namespace boost{
namespace container {
template<typename T>
using vector =
		::boost::container::vector<
			T,
			multi_thread_allocator_t<T>>;
}
}
}
}
#endif //HG_MT_BOOST_CONTAINER_VECTOR
