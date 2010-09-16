#include "Box.h"

#include <iostream>

using namespace ::hg;

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nSize, hg::TimeDirection nTimeDirection) :
referenceCount(new int(1)),
data(new Data(nX, nY, nXspeed, nYspeed, nSize, nTimeDirection))
{
}

Box::Box(const Box& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Box::~Box()
{
    decrementCount();
}

void Box::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Box& Box::operator=(const Box& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Box::operator!=(const Box& other) const
{
    return !(*this==other);
}

bool Box::operator==(const Box& other) const
{
	return data == other.data ||
        ((data->x == other.data->x)
        && (data->y == other.data->y)
        && (data->xspeed == other.data->xspeed) 
        && (data->yspeed == other.data->yspeed)
        && (data->timeDirection == other.data->timeDirection) 
        && (data->size == other.data->size));
}

bool Box::operator<(const Box& other) const
{
    if (data != other.data) {
        if (data->x == other.data->x)
        {
            if (data->y == other.data->y)
            {
                if (data->xspeed == other.data->xspeed)
                {
                    if (data->yspeed == other.data->yspeed)
                    {
                        if (data->timeDirection == other.data->timeDirection)
                        {
                            if (data->size == other.data->size)
                            {
                                return false;
                            }
                            else
                            {
                                return (data->size < other.data->size);
                            }
                        }
                        else
                        {
                            return (data->timeDirection < other.data->timeDirection);
                        }
                    }
                    else
                    {
                        return (data->yspeed < other.data->yspeed);
                    }
                }
                else
                {
                    return (data->xspeed < other.data->xspeed);
                }
            }
            else
            {
                return (data->y < other.data->y);
            }
        }
        else
        {
            return (data->x < other.data->x);
        }
    }
    else {
        return false;
    }
}
