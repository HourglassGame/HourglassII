#ifndef HG_INPUT_H
#define HG_INPUT_H
namespace hg {
    class Input;
}

#include <boost/noncopyable.hpp>
#include <SFML/Window/Input.hpp>
#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

namespace hg {
    class Input : private boost::noncopyable {
    public:
        Input(const sf::Input& input);
        const InputList AsInputList() const;
    private:
        bool left;
        bool right;
        bool up;
        bool down;
        bool space;
        bool mouseLeft;
        bool mouseRight;
        int mouseX;
        int mouseY;
    };
}
#endif //HG_INPUT_H
