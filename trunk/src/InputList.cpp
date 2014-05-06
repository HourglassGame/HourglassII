#include "InputList.h"
namespace hg {
InputList::InputList() noexcept :
    left(),
    right(),
    up(),
    down(),
    portalUsed(),
    abilityUsed(),
    abilityCursor(NO_ABILITY),
    timeCursor(),
    xCursor(),
    yCursor()
{
}
InputList::InputList(
    bool left,
    bool right,
    bool up,
    bool down,
    bool portalUsed,
    bool abilityUsed,
    Ability abilityCursor,
    FrameID timeCursor,
    int xCursor,
    int yCursor) noexcept :
        left(left),
        right(right),
        up(up),
        down(down),
        portalUsed(portalUsed),
        abilityUsed(abilityUsed),
        abilityCursor(abilityCursor),
        timeCursor(timeCursor),
        xCursor(xCursor),
        yCursor(yCursor)
{
}
bool InputList::operator==(InputList const &o) const
{
    return left == o.left
        && right == o.right
        && up == o.up
        && down == o.down
        && portalUsed == o.portalUsed
        && abilityUsed == o.abilityUsed
        && abilityCursor == o.abilityCursor
        && timeCursor == o.timeCursor
        && xCursor == o.xCursor
        && yCursor == o.yCursor;
}
} //namespace hg
