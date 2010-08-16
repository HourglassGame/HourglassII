#ifndef HG_TOTAL_STATE_HASH_H
#define HG_TOTAL_STATE_HASH_H
#include <vector> //For size_t -- not sure what the correct way is
#include <boost/functional/hash.hpp>
#include "WorldState.h"
namespace hg {
    class TotalStateHash {
    public:
        TotalStateHash(const WorldState& newWorldState, unsigned int newStackTop) :
        totalStateHash_(0)
        {
            ::boost::hash_combine(totalStateHash_, newStackTop);
            ::boost::hash_combine(totalStateHash_, newWorldState);
        }
        inline bool operator==(const TotalStateHash& other) const 
        {
            return totalStateHash_ == other.totalStateHash_;
        }
        ::std::size_t totalStateHash_;
    };
}
#endif //HG_TOTAL_STATE_H
