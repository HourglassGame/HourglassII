#ifndef HG_MT_BOOST_CONTAINER_STABLE_VECTOR
#define HG_MT_BOOST_CONTAINER_STABLE_VECTOR
#include "../../../multi_thread_allocator.h"
#include <boost/container/stable_vector.hpp>
namespace hg {
namespace mt {
namespace boost{
namespace container {
template<typename T>
struct stable_vector {
	typedef
		::boost::container::stable_vector<
			T,
			typename multi_thread_allocator<T>::type
		> type;
};
}
}
}
}
#endif //HG_MT_BOOST_CONTAINER_STABLE_VECTOR
