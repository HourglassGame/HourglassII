#include "InputList.h"
namespace hg {
InputList::InputList(
    bool left,
    bool right,
    bool up,
    bool down,
    bool use,
    Ability ability,
    FrameID timeParam,
    int xParam,
    int yParam) :
        left(left),
        right(right),
        up(up),
        down(down),
        use(use),
        ability(ability),
        timeParam(timeParam),
        xParam(xParam),
        yParam(yParam)
{
}
}
