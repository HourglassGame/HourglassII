#include "Button.h"

using namespace ::hg;

Button::Button(int nX, int nY, int nxspeed, int nyspeed, int index, bool state, hg::TimeDirection nTimeDirection) :
referenceCount(new int(1)),
data(new Data(nX, nY, nxspeed, nyspeed, index, state, nTimeDirection))
{
}

Button::Button(const Button& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Button::~Button()
{
    decrementCount();
}

void Button::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Button& Button::operator=(const Button& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Button::operator!=(const Button& other) const
{
    return !(*this==other);
}

bool Button::operator==(const Button& other) const
{
	return data == other.data ||
        ((data->state == other.data->state)
        && (data->x == other.data->x)
        && (data->y == other.data->y)
        && (data->xspeed == other.data->xspeed)
        && (data->yspeed == other.data->yspeed)
        && (data->index == other.data->index)
        && (data->timeDirection == other.data->timeDirection));
}

bool Button::operator<(const Button& other) const
{
    return data->y < other.data->index;
}
