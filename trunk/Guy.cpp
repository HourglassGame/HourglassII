#include "Guy.h"
#include <limits>
namespace hg {
Guy::Guy(int x,
         int y,
         int xspeed,
         int yspeed,
         int width,
         int height,

         int illegalPortal,
         int relativeToPortal,
         bool supported,
         int supportedSpeed,

         bool facing,

         bool boxCarrying,
         int boxCarrySize,
         TimeDirection boxCarryDirection,
         int boxPauseLevel,

         TimeDirection timeDirection,
         int pauseLevel,
         std::size_t index) :
        x_(x),
        y_(y),
        xspeed_(xspeed),
        yspeed_(yspeed),
        width_(width),
        height_(height),

        illegalPortal_(illegalPortal),
        relativeToPortal_(relativeToPortal),
        supported_(supported),
        supportedSpeed_(supportedSpeed),

        facing_(facing),

        boxCarrying_(boxCarrying),
        boxCarrySize_(boxCarrySize),
        boxCarryDirection_(boxCarryDirection),
        boxPauseLevel_(boxPauseLevel),

        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel),
        index_(index)
{
}

Guy::Guy(const Guy& other, int copyType) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),

        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        supported_(other.supported_),
        supportedSpeed_(other.supportedSpeed_),

        facing_(other.facing_),

        boxCarrying_(other.boxCarrying_),
        boxCarrySize_(other.boxCarrySize_),
        boxCarryDirection_(other.boxCarryDirection_),
        boxPauseLevel_(other.boxPauseLevel_),

        timeDirection_(other.timeDirection_),
        pauseLevel_(other.pauseLevel_),
        index_(other.index_)
{
	if (copyType == 1)
	{
		index_ = std::numeric_limits<std::size_t>::max();
		pauseLevel_ = pauseLevel_ + 1;
	}
}

Guy::Guy(const Guy& other, TimeDirection timeDirection, int pauseLevel) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),

        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        supported_(other.supported_),
        supportedSpeed_(other.supportedSpeed_),

        facing_(other.facing_),

        boxCarrying_(other.boxCarrying_),
        boxCarrySize_(other.boxCarrySize_),
        boxCarryDirection_(other.boxCarryDirection_),
        boxPauseLevel_(other.boxPauseLevel_),

        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel),
        index_(other.index_)
{
}

Guy::Guy(const Guy& other) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),

        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        supported_(other.supported_),
        supportedSpeed_(other.supportedSpeed_),

        facing_(other.facing_),

        boxCarrying_(other.boxCarrying_),
        boxCarrySize_(other.boxCarrySize_),
        boxCarryDirection_(other.boxCarryDirection_),
        boxPauseLevel_(other.boxPauseLevel_),

        timeDirection_(other.timeDirection_),
        pauseLevel_(other.pauseLevel_),
        index_(other.index_)
{
}

Guy& Guy::operator=(const Guy& other)
{
    x_ = other.x_;
    y_ = other.y_;
    xspeed_ = other.xspeed_;
    yspeed_ = other.yspeed_;
    width_ = other.width_;
    height_ = other.height_;

    illegalPortal_ = other.illegalPortal_;
    relativeToPortal_ = other.relativeToPortal_;
    supported_ = other.supported_;
    supportedSpeed_ = other.supportedSpeed_;

    facing_ = other.facing_;

    boxCarrying_ = other.boxCarrying_;
    boxCarrySize_ = other.boxCarrySize_;
    boxCarryDirection_ = other.boxCarryDirection_;
    boxPauseLevel_ = other.boxPauseLevel_;

    timeDirection_ = other.timeDirection_;
    pauseLevel_ = other.pauseLevel_;
    index_ = other.index_;
    return *this;
}

bool Guy::operator!=(const Guy& other) const
{
    return !(*this==other);
}

bool Guy::operator==(const Guy& other) const
{
    return  (index_ == other.index_)
            && (x_ == other.x_)
            && (y_ == other.y_)
            && (xspeed_ == other.xspeed_)
            && (yspeed_ == other.yspeed_)
            && (width_ == other.width_)
            && (height_ == other.height_)
            && (illegalPortal_ == other.illegalPortal_)
            && (relativeToPortal_ == other.relativeToPortal_)
            && (supported_ == other.supported_)
            && (supportedSpeed_ == other.supportedSpeed_)
            && (facing_ == other.facing_)
            && (boxCarrying_ == other.boxCarrying_)
            && (boxCarrySize_ == other.boxCarrySize_)
            && (boxCarryDirection_ == other.boxCarryDirection_)
            && (boxPauseLevel_ == other.boxPauseLevel_)
            && (timeDirection_ == other.timeDirection_)
            && (pauseLevel_ == other.pauseLevel_);
}

bool Guy::operator<(const Guy& other) const
{
    return
    pauseLevel_ == other.pauseLevel_?
        index_ < other.index_:
        other.pauseLevel_ < pauseLevel_;
}
}//namespace hg