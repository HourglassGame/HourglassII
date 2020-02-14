#ifndef HG_INPUT_H
#define HG_INPUT_H

#include "hg/TimeEngine/ArrivalDepartures/Ability.h"

#include "hg/TimeEngine/ArrivalDepartures/InputList_fwd.h"
#include "GLFWWindow.h"

namespace hg {
enum class ActivePanel { NONE, WORLD, GLOBAL_TIME, PERSONAL_TIME };

class Input final {
public:
    Input();
    void updateState(
        GLFWWindow &windowglfw,
        ActivePanel const mousePanel,
        bool waitingForWave,
        int mouseXTimelineOffset, int mouseXOfEndOfTimeline, int mouseXOfEndOfPersonalTimeline,
        std::size_t personalTimelineLength, 
        int mouseOffX, int mouseOffY, double mouseScale
    );
    InputList AsInputList() const;
    FrameID getTimeCursor() const;
    void setTimelineLength(int timelineLength_) { this->timelineLength = timelineLength_; }

    bool getAbilityChanged() const { return abilityChanged; }

private:
    bool left;
    bool right;
    bool up;
    int down;
    int boxLeft;
    int boxRight;
    int space;
    int mouseLeft;
    Ability abilityCursor;
    bool abilityChanged;
    int mouseTimelinePosition;
    int mousePersonalTimelinePosition;
    int mouseX;
    int mouseY;
    int timelineLength;
};
}
#endif //HG_INPUT_H
