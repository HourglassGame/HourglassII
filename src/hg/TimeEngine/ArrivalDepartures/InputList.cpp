#include "InputList.h"
namespace hg {
GuyInput::GuyInput() noexcept :
    moveLeft(),
    moveRight(),
    jump(),
    boxAction(),
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
    bool moveLeft,
    bool moveRight,
    bool jump,
    bool boxAction,
    bool boxLeft,
    bool boxRight,
    bool portalUsed,
    bool abilityUsed,
    Ability abilityCursor,
    FrameID timeCursor,
    int xCursor,
    int yCursor) noexcept :
        moveLeft(moveLeft),
        moveRight(moveRight),
        jump(jump),
        boxAction(boxAction),
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
    return moveLeft == o.moveLeft
        && moveRight == o.moveRight
        && jump == o.jump
        && boxAction == o.boxAction
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
