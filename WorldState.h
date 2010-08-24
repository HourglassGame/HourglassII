#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include "ArrivalDepartureMap.h"
#include "FrameID.h"
namespace hg {
    class WorldState {
    public:
        WorldState(const ArrivalDepartureMap& currentArrivalDepartures,
                   //::std::vector<Frame> newFrameUpdateList,
                   FrameID newCurrentPlayerFrame,
                   FrameID newNextPlayerFrame) :
        arrivalDepartures(currentArrivalDepartures),
        frameUpdateList(),
        nextPlayerFrame(newNextPlayerFrame),
        currentPlayerFrame(newCurrentPlayerFrame)
        {
        }
        bool operator==(const WorldState& other) const
        {
            return currentPlayerFrame == other.currentPlayerFrame 
            && frameUpdateList == other.frameUpdateList
            && arrivalDepartures == other.arrivalDepartures;
        }
        bool operator!=(const WorldState& other) const
        {
            return !(*this==other);
        }
        ArrivalDepartureMap arrivalDepartures;
        ::std::vector<FrameID> frameUpdateList;
        //The frame constaining the guy with the largest overall relative index - who has arrived but not yet departed
        FrameID nextPlayerFrame;
        //The frame containing the guy with the largest relative index who has both arrived and departed
        FrameID currentPlayerFrame;
    };
}
#endif //HG_WORLD_STATE_H
