#ifndef HG_BOOST_HASH_COMPARE
#define HG_BOOST_HASH_COMPARE
#include <cstdlib>
namespace hg {
template <typename T>
class BoostHashCompare
{
    public:
    BoostHashCompare(){}
    BoostHashCompare( const BoostHashCompare& ){}
    ~BoostHashCompare (){}
    bool equal( const T& j, const T& k ) const
    {
        return j == k;
    }
    size_t hash( const T& k ) const
    {
        return hash_value(k);
    }
};
}
#endif //HG_BOOST_HASH_COMPARE
