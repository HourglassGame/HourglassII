#include "Guy.h"
#include <limits>
namespace hg {
Guy::Guy(
    std::size_t index,
    int x, int y,
    int xspeed, int yspeed,
    int width, int height,
    int jumpSpeed,
    
    int illegalPortal,
    int arrivalBasis,
    bool supported,
    int supportedSpeed,

    mt::std::map<Ability, int> pickups,
    FacingDirection facing,

    bool boxCarrying,
    int boxCarrySize,
    TimeDirection boxCarryDirection,

    TimeDirection timeDirection,
    bool timePaused) :
        index(index),
        x(x), y(y),
        xspeed(xspeed), yspeed(yspeed),
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
