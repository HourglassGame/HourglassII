#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include <boost/functional/hash.hpp>
#include "ArrivalDepartureMap.h"
namespace hg {
    class WorldState {
    public:
        WorldState(const ArrivalDepartureMap& currentArrivalDepartures,
                   bool currentUpdateStartFirst,
                   unsigned int newCurrentPlayerFrame,
                   unsigned int newNextPlayerFrame) :
        arrivalDepartures(currentArrivalDepartures),
        updateStartFirst(currentUpdateStartFirst),
        currentPlayerFrame(newCurrentPlayerFrame),
        nextPlayerFrame(newNextPlayerFrame)
        {
        }
        bool operator==(const WorldState& other) const
        {
            return currentPlayerFrame == other.currentPlayerFrame 
            && nextPlayerFrame == other.nextPlayerFrame
            && updateStartFirst == other.updateStartFirst
            && arrivalDepartures == other.arrivalDepartures;
        }
        bool operator!=(const WorldState& other) const
        {
            return !(*this==other);
        }
        ArrivalDepartureMap arrivalDepartures;
        bool updateStartFirst;
        unsigned int currentPlayerFrame;
        unsigned int nextPlayerFrame;
    };
    
    ::std::size_t hash_value(const WorldState& toHash);
}
#endif //HG_WORLD_STATE_H
