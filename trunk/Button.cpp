#include "Button.h"
#include <limits>
#include <cassert>
namespace hg {
Button::Button(int x, int y, int xspeed, int yspeed, int width, int height, std::size_t index, bool state, TimeDirection timeDirection, int pauseLevel) :
        x_(x),
        y_(y),
        xspeed_(xspeed),
        yspeed_(yspeed),
        width_(width),
        height_(height),
        index_(index),
        state_(state),
        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel)
{
    assert(index_ != std::numeric_limits<std::size_t>::max()
           && "the max value is reserved for representing invalid/null indices");
}

Button::Button(int x, int y, int xspeed, int yspeed, const Button& other) :
    x_(x),
    y_(y),
    xspeed_(xspeed),
    yspeed_(yspeed),
    width_(other.width_),
    height_(other.height_),
    index_(other.index_),
    state_(other.state_),
    timeDirection_(other.timeDirection_),
    pauseLevel_(other.pauseLevel_)
{

}

Button::Button(const Button& other, TimeDirection timeDirection, int pauseLevel) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),
        index_(other.index_),
        state_(other.state_),
        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel)
{
}

Button::Button(const Button& other) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        width_(other.width_),
        height_(other.height_),
        index_(other.index_),
        state_(other.state_),
        timeDirection_(other.timeDirection_),
        pauseLevel_(other.pauseLevel_)
{
}

Button& Button::operator=(const Button& other)
{
    x_ = other.x_;
    y_ = other.y_;
    xspeed_ = other.xspeed_;
    yspeed_ = other.yspeed_;
    width_ = other.width_;
    height_ = other.height_;
    index_ = other.index_;
    state_ = other.state_;
    timeDirection_ = other.timeDirection_;
    pauseLevel_ = other.pauseLevel_;
    return *this;
}

bool Button::operator!=(const Button& other) const
{
    return !(*this==other);
}

bool Button::operator==(const Button& other) const
{
    return (state_ == other.state_)
           && (x_ == other.x_)
           && (y_ == other.y_)
           && (xspeed_ == other.xspeed_)
           && (yspeed_ == other.yspeed_)
           && (height_ == other.height_)
           && (width_ == other.width_)
           && (index_ == other.index_)
           && (timeDirection_ == other.timeDirection_)
           && (pauseLevel_ == other.pauseLevel_);
}

bool Button::operator<(const Button& other) const
{
    return index_ < other.index_;
}
}
