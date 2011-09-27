#include "Guy.h"
#include <limits>
namespace hg {
Guy::Guy(
    int x, int y,
    int xspeed, int yspeed,
    int width, int height,
    
    int illegalPortal,
    int arrivalBasis,
    bool supported,
    int supportedSpeed,

    mt::std::map<Ability, int>::type const& pickups,
    bool facing,

    bool boxCarrying,
    int boxCarrySize,
    TimeDirection boxCarryDirection,

    TimeDirection timeDirection,
    std::size_t index) :
        index_(index),
        x_(x), y_(y),
        xspeed_(xspeed), yspeed_(yspeed),
        width_(width), height_(height),

        illegalPortal_(illegalPortal),
        arrivalBasis_(arrivalBasis),
        supported_(supported),
        supportedSpeed_(supportedSpeed),

        pickups_(pickups),
        facing_(facing),

        boxCarrying_(boxCarrying),
        boxCarrySize_(boxCarrySize),
        boxCarryDirection_(boxCarryDirection),

        timeDirection_(timeDirection)
{
}

Guy::Guy(const Guy& other, TimeDirection timeDirection) :
        index_(other.index_),

        x_(other.x_), y_(other.y_),
        xspeed_(other.xspeed_), yspeed_(other.yspeed_),
        width_(other.width_), height_(other.height_),

        illegalPortal_(other.illegalPortal_),
        arrivalBasis_(other.arrivalBasis_),
        supported_(other.supported_),
        supportedSpeed_(other.supportedSpeed_),

        pickups_(other.pickups_),
        facing_(other.facing_),

        boxCarrying_(other.boxCarrying_),
        boxCarrySize_(other.boxCarrySize_),
        boxCarryDirection_(other.boxCarryDirection_),

        timeDirection_(timeDirection)
{
}

bool Guy::operator==(const Guy& other) const
{
    return (index_ == other.index_)
        && (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (width_ == other.width_)
        && (height_ == other.height_)
        && (illegalPortal_ == other.illegalPortal_)
        && (arrivalBasis_ == other.arrivalBasis_)
        && (supported_ == other.supported_)
        && (supportedSpeed_ == other.supportedSpeed_)
        && (pickups_ == other.pickups_)
        && (facing_ == other.facing_)
        && (boxCarrying_ == other.boxCarrying_)
        && (boxCarrySize_ == other.boxCarrySize_)
        && (boxCarryDirection_ == other.boxCarryDirection_)
        && (timeDirection_ == other.timeDirection_);
}

bool Guy::operator<(const Guy& other) const
{
    return index_ < other.index_;
}
}//namespace hg
