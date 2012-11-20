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

    mt::std::map<Ability, int>::type const& pickups,
    FacingDirection::FacingDirection facing,

    bool boxCarrying,
    int boxCarrySize,
    TimeDirection boxCarryDirection,

    TimeDirection timeDirection) :
        index_(index),
        x_(x), y_(y),
        xspeed_(xspeed), yspeed_(yspeed),
        width_(width), height_(height),
        jumpSpeed_(jumpSpeed),

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

Guy::Guy(const Guy& o, TimeDirection timeDirection) :
        index_(o.index_),

        x_(o.x_), y_(o.y_),
        xspeed_(o.xspeed_), yspeed_(o.yspeed_),
        width_(o.width_), height_(o.height_),
        jumpSpeed_(o.jumpSpeed_),

        illegalPortal_(o.illegalPortal_),
        arrivalBasis_(o.arrivalBasis_),
        supported_(o.supported_),
        supportedSpeed_(o.supportedSpeed_),

        pickups_(o.pickups_),
        facing_(o.facing_),

        boxCarrying_(o.boxCarrying_),
        boxCarrySize_(o.boxCarrySize_),
        boxCarryDirection_(o.boxCarryDirection_),

        timeDirection_(timeDirection)

{
}

bool Guy::operator==(const Guy& o) const
{
    return index_ == o.index_
        && x_ == o.x_
        && y_ == o.y_
        && xspeed_ == o.xspeed_
        && yspeed_ == o.yspeed_
        && width_ == o.width_
        && height_ == o.height_
        && jumpSpeed_ == o.jumpSpeed_
        && illegalPortal_ == o.illegalPortal_
        && arrivalBasis_ == o.arrivalBasis_
        && supported_ == o.supported_
        && supportedSpeed_ == o.supportedSpeed_
        && pickups_ == o.pickups_
        && facing_ == o.facing_
        && boxCarrying_ == o.boxCarrying_
        && boxCarrySize_ == o.boxCarrySize_
        && boxCarryDirection_ == o.boxCarryDirection_
        && timeDirection_ == o.timeDirection_;
}
bool Guy::operator<(const Guy& o) const
{
    return index_ < o.index_;
}
}//namespace hg
