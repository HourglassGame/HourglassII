#include "InputList.h"
namespace hg {
GuyInput::GuyInput() noexcept :
    left(),
    right(),
    up(),
    down(),
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
bool GuyInput::operator==(GuyInput const &o) const noexcept
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
