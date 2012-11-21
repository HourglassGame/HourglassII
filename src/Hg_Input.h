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
    void updateState(const sf::Input& input, int mouseXOfEndOfTimeline, double mouseScale);
    InputList AsInputList() const;
    void setTimelineLength(int newTimelineLength) { timelineLength = newTimelineLength; }
private:
    bool left;
    bool right;
    bool up;
    int q;
    int w;
    int down;
    int space;
    int mouseLeft;
    int mouseRight;
    int mouseX;
    int mouseY;
    int mouseTimelinePosition;
    int timelineLength;
};
}
#endif //HG_INPUT_H
