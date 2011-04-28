#include "Portal.h"
#include <limits>
#include <cassert>
namespace hg {
Portal::Portal(int x,
               int y,
               int xspeed,
               int yspeed,
               int width,
               int height,
               std::size_t index,
               TimeDirection timeDirection,
               int pauseLevel,
               int charges,
               bool active,
               int xDestination,
               int yDestination,
               int destinationIndex,
               int timeDestination,
               bool relativeTime) :
x_(x),
y_(y),
xspeed_(xspeed),
yspeed_(yspeed),
width_(width),
height_(height),
index_(index),
timeDirection_(timeDirection),
pauseLevel_(pauseLevel),
charges_(charges),
active_(active),
xDestination_(xDestination),
yDestination_(yDestination),
destinationIndex_(destinationIndex),
timeDestination_(timeDestination),
relativeTime_(relativeTime)
{
    assert(index_ != std::numeric_limits<std::size_t>::max() 
           && "the max value is reserved for representing invalid/null indices");
}

Portal::Portal(const Portal& other, hg::TimeDirection timeDirection, int pauseLevel) :
x_(other.x_),
y_(other.y_),
xspeed_(other.xspeed_),
yspeed_(other.yspeed_),
width_(other.width_),
height_(other.height_),
index_(other.index_),
timeDirection_(timeDirection),
pauseLevel_(pauseLevel),
charges_(other.charges_),
active_(other.active_),
xDestination_(other.xDestination_),
yDestination_(other.yDestination_),
destinationIndex_(other.destinationIndex_),
timeDestination_(other.timeDestination_),
relativeTime_(other.relativeTime_)
{
}

Portal::Portal(const Portal& other) :
x_(other.x_),
y_(other.y_),
xspeed_(other.xspeed_),
yspeed_(other.yspeed_),
width_(other.width_),
height_(other.height_),
index_(other.index_),
timeDirection_(other.timeDirection_),
pauseLevel_(other.pauseLevel_),
charges_(other.charges_),
active_(other.active_),
xDestination_(other.xDestination_),
yDestination_(other.yDestination_),
destinationIndex_(other.destinationIndex_),
timeDestination_(other.timeDestination_),
relativeTime_(other.relativeTime_)
{
}

Portal& Portal::operator=(const Portal& other)
{
    x_ = other.x_;
    y_ = other.y_;
    xspeed_ = other.xspeed_;
    yspeed_ = other.yspeed_;
    width_ = other.width_;
    height_ = other.height_;
    index_ = other.index_;
    timeDirection_ = other.timeDirection_;
    pauseLevel_ = other.pauseLevel_;
    charges_ = other.charges_;
    active_ = other.active_;
    xDestination_ = other.xDestination_;
    yDestination_ = other.yDestination_;
    destinationIndex_ = other.destinationIndex_;
    timeDestination_ = other.timeDestination_;
    relativeTime_ = other.relativeTime_;
    return *this;
}

bool Portal::operator!=(const Portal& other) const
{
    return !(*this==other);
}

bool Portal::operator==(const Portal& other) const
{
    return (index_ == other.index_)
        && (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (pauseLevel_ == other.pauseLevel_)
        && (charges_ == other.charges_)
        && (active_ == other.active_)
        && (destinationIndex_ == other.destinationIndex_)
        && (timeDestination_ == other.timeDestination_)
        && (relativeTime_ == other.relativeTime_)
        && (xDestination_  == other.xDestination_)
        && (yDestination_ == other.yDestination_)
        && (width_ == other.width_)
        && (height_ == other.height_);
}

bool Portal::operator<(const Portal& other) const
{
    return index_ < other.index_;
}
}
