#ifndef HG_MT_BOOST_UNORDERED_MAP_HPP
#define HG_MT_BOOST_UNORDERED_MAP_HPP
#include "../../multi_thread_allocator.h"
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
struct unordered_map {
	typedef
		::boost::unordered_map<
			Key,
			Mapped,
			Hash,
			Pred,
			typename multi_thread_allocator<
				::std::pair<Key const, Mapped>
			>::type
		> type;
};
}
}
}
#endif //HG_MT_BOOST_UNORDERED_MAP_HPP
