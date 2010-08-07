/*
 *  WorldState.h
 *  HourglassIIGameSFML
 *
 *  Created by Evan Wallace on 5/08/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include "ArrivalDepartureMap.h"
namespace hg {
    class WorldState {
    public:
        WorldState(const hg::ArrivalDepartureMap& currentArrivalDepartures,
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
        hg::ArrivalDepartureMap arrivalDepartures;
        bool updateStartFirst;
        int currentPlayerFrame;
        int nextPlayerFrame;
    };
}
#endif //HG_WORLD_STATE_H
