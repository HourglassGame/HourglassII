#ifndef HG_MT_BOOST_CONTAINER_MAP
#define HG_MT_BOOST_CONTAINER_MAP
#include "../../../multi_thread_allocator.h"
#include <boost/container/map.hpp>
namespace hg {
namespace mt {
namespace boost {
namespace container {
template<
	typename Key,
	typename T,
	typename Compare = ::std::less<Key> >
struct map {
	typedef
		::boost::container::map<
			Key,
			T,
			Compare,
			typename multi_thread_allocator<
				::std::pair<Key const, T>
			>::type
		> type;
};
}
}
}
}
#endif //HG_MT_BOOST_CONTAINER_MAP
