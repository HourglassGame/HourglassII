#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include "ArrivalDepartureMap.h"
namespace hg {
    class WorldState {
    public:
        WorldState(const ArrivalDepartureMap& currentArrivalDepartures,
                   bool currentUpdateStartFirst,
                   int newCurrentPlayerFrame,
                   int newNextPlayerFrame) :
        arrivalDepartures(currentArrivalDepartures),
        updateStartFirst(currentUpdateStartFirst),
        currentPlayerFrame(newCurrentPlayerFrame),
        nextPlayerFrame(newNextPlayerFrame)
        {}
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
        int currentPlayerFrame;
        int nextPlayerFrame;
    };
}
#endif //HG_WORLD_STATE_H
