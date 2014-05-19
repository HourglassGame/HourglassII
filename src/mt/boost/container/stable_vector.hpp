#ifndef HG_MT_BOOST_CONTAINER_STABLE_VECTOR
#define HG_MT_BOOST_CONTAINER_STABLE_VECTOR
#include "../../../multi_thread_allocator.h"
#include <boost/container/stable_vector.hpp>
namespace hg {
namespace mt {
namespace boost{
namespace container {
template<typename T>
using stable_vector =
		::boost::container::stable_vector<
			T,
			multi_thread_allocator_t<T>>;
}
}
}
}
#endif //HG_MT_BOOST_CONTAINER_STABLE_VECTOR
