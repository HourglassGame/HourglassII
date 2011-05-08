#ifndef HG_INPUT_H
#define HG_INPUT_H

#include "Ability.h"
#include <SFML/Window/Input.hpp>
#include "InputList_fwd.h"

namespace hg {
class Input {
public:
    Input();
    void updateState(const sf::Input& input);
    const InputList AsInputList() const;
private:
    bool left;
    bool right;
    bool up;
    int down;
    bool space;
    int mouseLeft;
    int mouseRight;
    int mouseX;
    int mouseY;
    int pause;
};
}
#endif //HG_INPUT_H
