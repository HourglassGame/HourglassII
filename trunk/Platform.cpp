#include "Platform.h"
#include <limits>
#include <cassert>
namespace hg {
Platform::Platform(
    int x, int y,
    int xspeed, int yspeed,
    int width, int height,
    size_t index,
    TimeDirection timeDirection) :
        index_(index),
        x_(x),
        y_(y),
        xspeed_(xspeed),
        yspeed_(yspeed),
        width_(width),
        height_(height),
        timeDirection_(timeDirection)
{
    assert(index_ != std::numeric_limits<size_t>::max()
           && "the max value is reserved for representing invalid/null indices");
}

Platform::Platform(const Platform& other, hg::TimeDirection timeDirection) :
        index_(other.index_),
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),
        timeDirection_(timeDirection)
{
}

bool Platform::operator==(const Platform& other) const
{
    return (index_ == other.index_)
        && (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (width_ == other.width_)
        && (height_ == other.height_)
        && (timeDirection_ == other.timeDirection_);
}

bool Platform::operator<(const Platform& other) const
{
    return index_ < other.index_;
}
}
