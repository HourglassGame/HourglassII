#include "Guy.h"
#include <limits>
namespace hg {
Guy::Guy(
    std::size_t index,
    int x, int y,
    int xspeed, int yspeed,
    int walkSpeed,
    int jumpHold,
    GuyAction action,
    int width, int height,
    int jumpSpeed,
    
    int illegalPortal,
    int arrivalBasis,
    int supported,
    int supportedSpeed,

    Pickups pickups,
    FacingDirection facing,

    BoxType boxCarrying,
    int boxCarrySize,
    TimeDirection boxCarryDirection,

    TimeDirection timeDirection,
    bool timePaused) :
        index(index),
        x(x), y(y),
        xspeed(xspeed), yspeed(yspeed),
        walkSpeed(walkSpeed),
        jumpHold(jumpHold),
        action(action),
        width(width), height(height),
        jumpSpeed(jumpSpeed),

        illegalPortal(illegalPortal),
        arrivalBasis(arrivalBasis),
        supported(supported),
        supportedSpeed(supportedSpeed),

        pickups(std::move(pickups)),
        facing(facing),

        boxCarrying(boxCarrying),
        boxCarrySize(boxCarrySize),
        boxCarryDirection(boxCarryDirection),

        timeDirection(timeDirection),
        timePaused(timePaused)
{
}

bool Guy::operator==(Guy const &o) const
{
    return equality_tuple() == o.equality_tuple();
}
bool Guy::operator<(Guy const &o) const
{
    return index < o.index;
}
}//namespace hg
