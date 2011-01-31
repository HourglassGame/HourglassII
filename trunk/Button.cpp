#include "Button.h"

namespace hg {
Button::Button(int x, int y, int xspeed, int yspeed, int index, bool state, TimeDirection timeDirection, int pauseLevel) :
x_(x),
y_(y),
xspeed_(xspeed),
yspeed_(yspeed),
index_(index),
state_(state),
timeDirection_(timeDirection),
pauseLevel_(pauseLevel)
{
}

Button::Button(const Button& other, hg::TimeDirection timeDirection, int pauseLevel) :
x_(other.x_),
y_(other.y_),
xspeed_(other.xspeed_),
yspeed_(other.yspeed_),
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
index_(other.index_),
state_(other.state_),
timeDirection_(other.timeDirection_),
pauseLevel_(other.pauseLevel_)
{
}

Button::~Button()
{
}

Button& Button::operator=(const Button& other)
{
    x_ = other.x_;
    y_ = other.y_;
    xspeed_ = other.xspeed_;
    yspeed_ = other.yspeed_;
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
        && (index_ == other.index_)
        && (timeDirection_ == other.timeDirection_)
        && (pauseLevel_ == other.pauseLevel_);
}

bool Button::operator<(const Button& other) const
{
    return index_ < other.index_;
}
}
