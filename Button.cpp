#include "Button.h"
#include <limits>
#include <cassert>
namespace hg {
Button::Button(
    int x, int y,
    int xspeed, int yspeed,
    int width, int height,
    std::size_t index,
    bool state,
    TimeDirection timeDirection) :
        x_(x),y_(y),
        xspeed_(xspeed),yspeed_(yspeed),
        width_(width),height_(height),
        index_(index),
        state_(state),
        timeDirection_(timeDirection)
{
    assert(index_ != std::numeric_limits<std::size_t>::max()
           && "the max value is reserved for representing invalid/null indices");
}

Button::Button(int x, int y, int xspeed, int yspeed, const Button& other) :
    x_(x), y_(y),
    xspeed_(xspeed), yspeed_(yspeed),
    width_(other.width_), height_(other.height_),
    index_(other.index_),
    state_(other.state_),
    timeDirection_(other.timeDirection_)
{

}

Button::Button(const Button& other, TimeDirection timeDirection) :
        x_(other.x_), y_(other.y_),
        xspeed_(other.xspeed_), yspeed_(other.yspeed_),
        width_(other.width_), height_(other.height_),
        index_(other.index_),
        state_(other.state_),
        timeDirection_(timeDirection)
{
}

bool Button::operator==(const Button& other) const
{
    return (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (width_ == other.width_)
        && (height_ == other.height_)
        && (index_ == other.index_)
        && (state_ == other.state_)
        && (timeDirection_ == other.timeDirection_);
}

bool Button::operator<(const Button& other) const
{
    return index_ < other.index_;
}
}
