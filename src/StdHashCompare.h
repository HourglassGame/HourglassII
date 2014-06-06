#ifndef HG_STD_HASH_COMPARE
#define HG_STD_HASH_COMPARE
#include <functional>
#include <cstddef>
namespace hg {
template <typename T>
struct StdHashCompare
{
    bool equal(T const &j, T const &k) const
    {
        return std::equal_to<T>()(j, k);
    }
    std::size_t hash(T const &k) const
    {
        return std::hash<T>()(k);
    }
};
}
#endif //HG_STD_HASH_COMPARE
