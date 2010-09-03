#include <cassert>
#include "Guy.h"

using namespace hg;

Guy::Guy(int nX, 
         int nY, 
         int nXspeed, 
         int nYspeed, 
         int nWidth, 
         int nHeight,
         bool nBoxCarrying, 
         int nBoxCarrySize,
         TimeDirection nBoxCarryDirection,
         TimeDirection nTimeDirection, 
         unsigned int nRelativeIndex,
         int nSubimage) :
referenceCount(new int(1)),
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight,
              nBoxCarrying,nBoxCarrySize,nBoxCarryDirection,
              nTimeDirection,nRelativeIndex,nSubimage))
{
}
Guy::Guy(const Guy& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Guy::~Guy()
{
    decrementCount();
}

void Guy::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Guy& Guy::operator=(const Guy& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Guy::operator!=(const Guy& other) const
{
    return !(*this==other);
}

bool Guy::operator==(const Guy& other) const
{
	return data == other.data || 
        ((data->relativeIndex == other.data->relativeIndex) 
    && (data->x == other.data->x) 
    && (data->y == other.data->y) 
    && (data->xspeed == other.data->xspeed) 
    && (data->yspeed == other.data->yspeed) 
    && (data->width == other.data->width) 
    && (data->height == other.data->height) 
    && (data->boxCarrying == other.data->boxCarrying) 
    && (data->timeDirection == other.data->timeDirection) 
    && (data->subimage == other.data->subimage)
    && (data->boxCarryDirection == other.data->boxCarryDirection) 
    && (data->boxCarrySize == other.data->boxCarrySize));
}

bool Guy::operator<(const Guy& other) const
{
    if (data != other.data) {
        if (data->relativeIndex == other.data->relativeIndex)
        {
            if (data->x == other.data->x)
            {
                if (data->y == other.data->y)
                {
                    if (data->xspeed == other.data->xspeed)
                    {
                        if (data->yspeed == other.data->yspeed)
                        {
                            if (data->width == other.data->width)
                            {
                                if (data->height == other.data->height)
                                {
                                    if (data->boxCarrying == other.data->boxCarrying)
                                    {
                                        if (data->timeDirection == other.data->timeDirection)
                                        {
                                            if (data->subimage == other.data->subimage)
                                            {
                                                if (data->boxCarryDirection == other.data->boxCarryDirection)
                                                {
                                                    if (data->boxCarrySize == other.data->boxCarrySize)
                                                    {
                                                        return false;
                                                    }
                                                    else
                                                    {
                                                        return (data->boxCarrySize < other.data->boxCarrySize);
                                                    }
                                                }
                                                else
                                                {
                                                    return (data->boxCarryDirection < other.data->boxCarryDirection);
                                                }
                                            }
                                            else
                                            {
                                                return (data->subimage < other.data->subimage);
                                            }
                                        }
                                        else
                                        {
                                            return (data->timeDirection < other.data->timeDirection);
                                        }
                                    }
                                    else
                                    {
                                        return (data->boxCarrying < other.data->boxCarrying);
                                    }
                                }
                                else
                                {
                                    return (data->height < other.data->height);
                                }
                            }
                            else
                            {
                                return (data->width < other.data->width);
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
        else
        {
            return (data->relativeIndex < other.data->relativeIndex);
        }
    }
    else {
        return false;
    }
}
