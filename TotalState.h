#ifndef HG_TOTAL_STATE_H
#define HG_TOTAL_STATE_H
#include <vector>
#include "WorldState.h"
namespace hg {
    class TotalState {
    public:
        TotalState(const WorldState& newWorldState, const ::std::vector<unsigned int>& newStackState) :
        worldState(newWorldState),
        stackState(newStackState)
        {}
        inline bool operator==(const TotalState& other) const 
        {
            return stackState.back() == other.stackState.back()
                    && worldState == other.worldState;
        }
        WorldState worldState;
        ::std::vector<unsigned int> stackState;
    };
}
#endif //HG_TOTAL_STATE_H
