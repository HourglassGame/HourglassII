#ifndef HG_BOOST_HASH_COMPARE
#define HG_BOOST_HASH_COMPARE
#include <boost/functional/hash.hpp>
#include <functional>
#include <cstddef>
namespace hg {
template <typename T>
struct BoostHashCompare
{
    bool equal(T const& j, T const& k) const
    {
        return std::equal_to<T>()(j, k);
    }
    std::size_t hash(T const& k) const
    {
        return boost::hash<T>()(k);
    }
};
}
#endif //HG_BOOST_HASH_COMPARE
