#include "InputList.h"
namespace hg {
GuyInput::GuyInput() noexcept :
    left(),
    right(),
    up(),
    down(),
    boxLeft(),
    boxRight(),
    portalUsed(),
    abilityUsed(),
    abilityCursor(Ability::NO_ABILITY),
    timeCursor(),
    xCursor(),
    yCursor()
{
}
GuyInput::GuyInput(
    bool left,
    bool right,
    bool up,
    bool down,
    bool boxLeft,
    bool boxRight,
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
        boxLeft(boxLeft),
        boxRight(boxRight),
        portalUsed(portalUsed),
        abilityUsed(abilityUsed),
        abilityCursor(abilityCursor),
        timeCursor(timeCursor),
        xCursor(xCursor),
        yCursor(yCursor)
{
}
bool GuyInput::operator==(GuyInput const &o) const noexcept
{
    return left == o.left
        && right == o.right
        && up == o.up
        && down == o.down
        && boxLeft == o.boxLeft
        && boxRight == o.boxRight
        && portalUsed == o.portalUsed
        && abilityUsed == o.abilityUsed
        && abilityCursor == o.abilityCursor
        && timeCursor == o.timeCursor
        && xCursor == o.xCursor
        && yCursor == o.yCursor;
}
} //namespace hg
