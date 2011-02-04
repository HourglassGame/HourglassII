#ifndef HG_BOOST_HASH_COMPARE
#define HG_BOOST_HASH_COMPARE
#include <boost/functional/hash.hpp>
#include <cstdlib>
namespace hg {
template <typename T>
struct BoostHashCompare
{
    bool equal( const T& j, const T& k ) const
    {
        return j == k;
    }
    size_t hash( const T& k ) const
    {
        return boost::hash<T>()(k);
    }
};
}
#endif //HG_BOOST_HASH_COMPARE
