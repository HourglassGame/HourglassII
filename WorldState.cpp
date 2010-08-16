#include "WorldState.h"

::std::size_t hg::hash_value(const WorldState& toHash) {
    ::std::size_t seed(0);
    ::boost::hash_combine(seed, toHash.arrivalDepartures);
    ::boost::hash_combine(seed, toHash.updateStartFirst);
    ::boost::hash_combine(seed, toHash.currentPlayerFrame);
    ::boost::hash_combine(seed, toHash.nextPlayerFrame);
    return seed;
}

