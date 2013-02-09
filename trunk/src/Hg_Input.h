#ifndef HG_INPUT_H
#define HG_INPUT_H

#include "Ability.h"
#include <SFML/Window/Input.hpp>
#include "InputList_fwd.h"
#include <cstdlib>
namespace hg {
class Input {
public:
    Input();
    void updateState(sf::Input const& input, int mouseXOfEndOfTimeline, double mouseScale);
    InputList AsInputList() const;
    void setTimelineLength(int timelineLength) { this->timelineLength = timelineLength; }
private:
    bool left;
    bool right;
    bool up;
    int down;
    int space;
    int mouseLeft;
    Ability abilityCursor;
    int mouseTimelinePosition;
    int mouseX;
    int mouseY;
    int timelineLength;
};
}
#endif //HG_INPUT_H