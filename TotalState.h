#ifndef HG_TOTAL_STATE_H
#define HG_TOTAL_STATE_H
#include <vector>
#include "WorldState.h"
namespace hg {
    class TotalState {
    public:
        //These parameters must be COPIED
        TotalState(const WorldState& newWorldState, const ::std::vector<int>& newStackState) :
        worldState(newWorldState),
        stackState(newStackState)
        {}
        inline bool operator==(const TotalState& other) const 
        {
            return stackState.back() == other.stackState.back()
                    && worldState == other.worldState;
        }
        WorldState worldState;
        ::std::vector<int> stackState;
    };
}
#endif //HG_TOTAL_STATE_H