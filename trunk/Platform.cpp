#include "Platform.h"
#include <limits>
#include <cassert>
namespace hg {
Platform::Platform(int x, int y, int xspeed, int yspeed, 
                   int width, int height, size_t index, 
                   TimeDirection timeDirection, int pauseLevel) :
x_(x),
y_(y),
xspeed_(xspeed),
yspeed_(yspeed),
width_(width),
height_(height),
index_(index),
timeDirection_(timeDirection),
pauseLevel_(pauseLevel)
{
    assert(index_ != std::numeric_limits<size_t>::max() 
           && "the max value is reserved for representing invalid/null indices");
}

Platform::Platform(const Platform& other, hg::TimeDirection timeDirection, int pauseLevel) :
x_(other.x_),
y_(other.y_),
xspeed_(other.xspeed_),
yspeed_(other.yspeed_),
width_(other.width_),
height_(other.height_),
index_(other.index_),
timeDirection_(timeDirection),
pauseLevel_(pauseLevel)
{
}

Platform::Platform(const Platform& other) :
x_(other.x_),
y_(other.y_),
xspeed_(other.xspeed_),
yspeed_(other.yspeed_),
width_(other.width_),
height_(other.height_),
index_(other.index_),
timeDirection_(other.timeDirection_),
pauseLevel_(other.pauseLevel_)
{
}

Platform& Platform::operator=(const Platform& other)
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
    return *this;
}

bool Platform::operator!=(const Platform& other) const
{
    return !(*this==other);
}

bool Platform::operator==(const Platform& other) const
{
	return (index_ == other.index_)
        && (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (pauseLevel_ == other.pauseLevel_)
        && (width_ == other.width_)
        && (height_ == other.height_);
}

bool Platform::operator<(const Platform& other) const
{
    return index_ < other.index_;
}
}
