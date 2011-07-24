#ifndef HG_MT_BOOST_CONTAINER_VECTOR_HPP
#define HG_MT_BOOST_CONTAINER_VECTOR_HPP
#include "../../../multi_thread_allocator.h"
#include <boost/container/vector.hpp>
namespace hg {
namespace mt {
namespace boost{
namespace container{
template<typename T>
struct vector {
	typedef
		::boost::container::vector<
			T,
			typename multi_thread_allocator<T>::type
		> type;
};
}
}
}
}
#endif //HG_MT_BOOST_CONTAINER_VECTOR_HPP
