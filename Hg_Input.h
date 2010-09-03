#ifndef HG_INPUT_H
#define HG_INPUT_H
namespace hg {
    class Input;
}

#include <boost/noncopyable.hpp>
#include <SFML/Window/Input.hpp>
#include "InputList.h"
#include "Ability.h"

namespace hg {
    class Input {
    public:
        Input();
        Input(const sf::Input& input, const Input& previousInput);
        const InputList AsInputList() const;
    private:
        bool left;
        bool right;
        bool up;
        int down;
        bool space;
        int mouseLeft;
        bool mouseRight;
        int mouseX;
        int mouseY;
    };
}
#endif //HG_INPUT_H
