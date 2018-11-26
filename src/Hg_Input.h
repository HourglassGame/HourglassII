#ifndef HG_INPUT_H
#define HG_INPUT_H

#include "Ability.h"

#include "RenderWindow.h"
#include "InputList_fwd.h"

namespace hg {
enum class ActivePanel { NONE, WORLD, GLOBAL_TIME, PERSONAL_TIME };

class Input final {
public:
    Input();
    void updateState(
        hg::RenderWindow::InputState const &keyState,
        ActivePanel const mousePanel,
        int mouseXTimelineOffset, int mouseXOfEndOfTimeline, int mouseXOfEndOfPersonalTimeline,
        std::size_t personalTimelineLength, 
        int mouseOffX, int mouseOffY, double mouseScale
    );
    InputList AsInputList() const;
    void setTimelineLength(int timelineLength_) { this->timelineLength = timelineLength_; }
private:
    bool left;
    bool right;
    bool up;
    int down;
    int space;
    int mouseLeft;
    Ability abilityCursor;
    int mouseTimelinePosition;
    int mousePersonalTimelinePosition;
    int mouseX;
    int mouseY;
    int timelineLength;
};
}
#endif //HG_INPUT_H
