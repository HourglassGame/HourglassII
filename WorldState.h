#ifndef HG_WORLD_STATE_H
#define HG_WORLD_STATE_H
#include <vector>
#include "TimelineState.h"
#include "FrameID.h"
namespace hg {
    class WorldState {
    public:
        WorldState(const TimelineState& currentTimeline,
                   //::std::vector<Frame> newFrameUpdateList,
                   FrameID newCurrentPlayerFrame,
                   FrameID newNextPlayerFrame) :
        timeline(currentTimeline),
        frameUpdateList(),
        nextPlayerFrame(newNextPlayerFrame),
        currentPlayerFrame(newCurrentPlayerFrame)
        {
        }
        bool operator==(const WorldState& other) const
        {
            return currentPlayerFrame == other.currentPlayerFrame 
            && frameUpdateList == other.frameUpdateList
            && timeline == other.timeline;
        }
        bool operator!=(const WorldState& other) const
        {
            return !(*this==other);
        }
        TimelineState timeline;
        ::std::vector<FrameID> frameUpdateList;
        //The frame constaining the guy with the largest overall relative index - who has arrived but not yet departed
        FrameID nextPlayerFrame;
        //The frame containing the guy with the largest relative index who has both arrived and departed
        FrameID currentPlayerFrame;
    };
}
#endif //HG_WORLD_STATE_H
